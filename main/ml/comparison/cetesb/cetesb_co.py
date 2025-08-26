from datetime import datetime
import pandas as pd
from pathlib import Path
from sklearn.metrics import mean_absolute_error, mean_squared_error
import numpy as np
import matplotlib.pyplot as plt

# Caminhos
base_path = Path(__file__).resolve().parents[2] / "data" / "processed"
sensores_path = base_path / "sensor_readings_corrigido.csv"
cetesb_path   = base_path / "cetesb_co_congonhas_formatado.csv"

# Ler arquivos
df_sensores = pd.read_csv(sensores_path)
df_cetesb   = pd.read_csv(cetesb_path)

# Garantir numéricos
df_sensores["CO_ppm"] = pd.to_numeric(df_sensores["CO_ppm_corrigido"], errors="coerce")
df_cetesb["CO_ppm"]   = pd.to_numeric(df_cetesb["CO_ppm"], errors="coerce")

# Converter Data+Hora para datetime
df_sensores["datetime"] = pd.to_datetime(df_sensores["Data"] + " " + df_sensores["Hora"])

# Corrigir "24:00" para "00:00" do dia seguinte no CETESB
def corrigir_24h(row):
    if row['Hora'] == "24:00":
        data = pd.to_datetime(row['Data']) + pd.Timedelta(days=1)
        return data.strftime("%Y-%m-%d") + " 00:00"
    else:
        return row['Data'] + " " + row['Hora']

df_cetesb["datetime_str"] = df_cetesb.apply(corrigir_24h, axis=1)
df_cetesb["datetime"] = pd.to_datetime(df_cetesb["datetime_str"])
df_cetesb.drop(columns="datetime_str", inplace=True)

# Arredondar horário do sensor para a hora cheia
df_sensores["datetime"] = df_sensores["datetime"].dt.floor("h")

# Agrupar sensores por hora cheia e tirar média
df_sensores = df_sensores.groupby("datetime", as_index=False)["CO_ppm"].mean()

# Merge pelo datetime
df_merge = pd.merge(
    df_sensores, 
    df_cetesb, 
    on="datetime", 
    suffixes=("_sensores", "_cetesb")
)

# Remover NaNs
df_merge = df_merge.dropna(subset=["CO_ppm_sensores", "CO_ppm_cetesb"])

# Calcular métricas
mae  = mean_absolute_error(df_merge["CO_ppm_cetesb"], df_merge["CO_ppm_sensores"])
rmse = np.sqrt(mean_squared_error(df_merge["CO_ppm_cetesb"], df_merge["CO_ppm_sensores"]))
mape = np.mean(np.abs((df_merge["CO_ppm_cetesb"] - df_merge["CO_ppm_sensores"]) / df_merge["CO_ppm_cetesb"])) * 100
corr = df_merge["CO_ppm_cetesb"].corr(df_merge["CO_ppm_sensores"])

# Resultados no console
print("📊 Comparação Leituras Sensores vs CETESB")
print(f"Quantidade de pontos comparados: {len(df_merge)}")
print(f"MAE  (Erro Absoluto Médio): {mae:.4f} ppm")
print(f"RMSE (Raiz do Erro Quadrático Médio): {rmse:.4f} ppm")
print(f"MAPE (Erro Percentual Médio): {mape:.2f}%")
print(f"Correlação de Pearson: {corr:.4f}")

# 📈 Gráfico de comparação
plt.figure(figsize=(12,6))
plt.plot(df_merge["datetime"], df_merge["CO_ppm_cetesb"], label="CETESB", marker='o')
plt.plot(df_merge["datetime"], df_merge["CO_ppm_sensores"], label="Sensores", marker='x')
plt.xlabel("Data/Hora")
plt.ylabel("CO (ppm)")
plt.title("Comparação de CO - Sensores vs CETESB")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("comparacao_co.png", dpi=300)
