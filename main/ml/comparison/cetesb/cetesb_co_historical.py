from datetime import datetime
import pandas as pd
from pathlib import Path
from sklearn.metrics import mean_absolute_error, mean_squared_error
import numpy as np
import matplotlib.pyplot as plt

# ---- Caminhos ----
base_path = Path(__file__).resolve().parents[2] / "data" / "processed"
sensores_path = base_path / "sensor_readings_formatado.csv"
cetesb_path   = base_path / "cetesb_co_congonhas_formatado.csv"
output_csv    = base_path / "comparacao_corrigida.csv"

# ---- Ler dados ----
df_sensores = pd.read_csv(sensores_path)
df_cetesb   = pd.read_csv(cetesb_path)

# ---- Garantir numéricos ----
df_sensores["CO_ppm"] = pd.to_numeric(df_sensores["CO_ppm"], errors="coerce")
df_cetesb["CO_ppm"]   = pd.to_numeric(df_cetesb["CO_ppm"], errors="coerce")

# ---- Converter Data+Hora para datetime ----
df_sensores["datetime"] = pd.to_datetime(df_sensores["Data"] + " " + df_sensores["Hora"])
df_sensores["hour"] = df_sensores["datetime"].dt.hour

# Corrigir "24:00" na CETESB para "00:00" do dia seguinte
def corrigir_24h(row):
    if row['Hora'] == "24:00":
        data = pd.to_datetime(row['Data']) + pd.Timedelta(days=1)
        return data.strftime("%Y-%m-%d") + " 00:00"
    else:
        return row['Data'] + " " + row['Hora']

df_cetesb["datetime_str"] = df_cetesb.apply(corrigir_24h, axis=1)
df_cetesb["datetime"] = pd.to_datetime(df_cetesb["datetime_str"])
df_cetesb["hour"] = df_cetesb["datetime"].dt.hour
df_cetesb.drop(columns="datetime_str", inplace=True)

# ---- Média histórica horária ----
media_historica_sensores = df_sensores.groupby("hour")["CO_ppm"].mean()
media_historica_cetesb   = df_cetesb.groupby("hour")["CO_ppm"].mean()

# ---- Função de correção ----
def corrigir_por_hora(row, media_historica):
    media_atual_hora = media_historica[row["hour"]]
    if row["CO_ppm"] == 0:
        return row["CO_ppm"]
    fator = media_atual_hora / row["CO_ppm"]
    return row["CO_ppm"] * fator

# ---- Aplicar correção ----
df_sensores["CO_ppm_corrigido"] = df_sensores.apply(lambda r: corrigir_por_hora(r, media_historica_sensores), axis=1)
df_cetesb["CO_ppm_corrigido"]   = df_cetesb.apply(lambda r: corrigir_por_hora(r, media_historica_cetesb), axis=1)

# ---- Criar coluna de hora cheia ----
df_sensores["datetime_h"] = df_sensores["datetime"].dt.floor("h")
df_cetesb["datetime_h"]   = df_cetesb["datetime"].dt.floor("h")

# ---- Agrupar por hora cheia ----
df_sensores_grouped = df_sensores.groupby("datetime_h", as_index=False)["CO_ppm_corrigido"].mean()
df_cetesb_grouped   = df_cetesb.groupby("datetime_h", as_index=False)["CO_ppm_corrigido"].mean()

# ---- Merge para comparação ----
df_merge = pd.merge(
    df_sensores_grouped,
    df_cetesb_grouped,
    on="datetime_h",
    suffixes=("_sensores", "_cetesb")
)

# ---- Remover NaNs ----
df_merge = df_merge.dropna(subset=["CO_ppm_corrigido_sensores", "CO_ppm_corrigido_cetesb"])

# ---- Métricas de comparação ----
mae  = mean_absolute_error(df_merge["CO_ppm_corrigido_cetesb"], df_merge["CO_ppm_corrigido_sensores"])
rmse = np.sqrt(mean_squared_error(df_merge["CO_ppm_corrigido_cetesb"], df_merge["CO_ppm_corrigido_sensores"]))
mape = np.mean(np.abs((df_merge["CO_ppm_corrigido_cetesb"] - df_merge["CO_ppm_corrigido_sensores"]) /
                      df_merge["CO_ppm_corrigido_cetesb"])) * 100
corr = df_merge["CO_ppm_corrigido_cetesb"].corr(df_merge["CO_ppm_corrigido_sensores"])

print("📊 Comparação Leituras Sensores vs CETESB (Corrigido por Média Histórica)")
print(f"Quantidade de pontos comparados: {len(df_merge)}")
print(f"MAE  (Erro Absoluto Médio): {mae:.4f} ppm")
print(f"RMSE (Raiz do Erro Quadrático Médio): {rmse:.4f} ppm")
print(f"MAPE (Erro Percentual Médio): {mape:.2f}%")
# print(f"Correlação de Pearson: {corr:.4f}")

# ---- Salvar CSV final ----
df_merge.to_csv(output_csv, index=False)
print(f"CSV de comparação salvo em: {output_csv}")

# ---- Gráfico ----
plt.figure(figsize=(12,6))
plt.plot(df_merge["datetime_h"], df_merge["CO_ppm_corrigido_cetesb"], label="CETESB", marker='o')
plt.plot(df_merge["datetime_h"], df_merge["CO_ppm_corrigido_sensores"], label="Sensores", marker='x')
plt.xlabel("Data/Hora")
plt.ylabel("CO (ppm)")
plt.title("Comparação CO Corrigido por Média Histórica - Sensores vs CETESB")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(base_path / "comparacao_co_corrigido.png", dpi=300)
