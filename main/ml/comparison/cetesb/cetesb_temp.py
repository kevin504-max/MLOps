from datetime import datetime
import pandas as pd
from pathlib import Path
from sklearn.metrics import mean_absolute_error, mean_squared_error
import numpy as np
import matplotlib.pyplot as plt

# Caminhos
base_path = Path(__file__).resolve().parents[2] / "data" / "processed"
sensores_path = base_path / "sensor_readings_formatado.csv"        # arquivo dos sensores
cetesb_path   = base_path / "cetesb_sao_jose_rio_preto_formatado.csv"  # CETESB temperatura

# Ler arquivos
df_sensores = pd.read_csv(sensores_path)
df_cetesb   = pd.read_csv(cetesb_path)

# Colunas de interesse
col_temp_sensores = "Temperature_C"
col_temp_cetesb   = "Temperature_C"

# Garantir que sejam numéricos
df_sensores[col_temp_sensores] = pd.to_numeric(df_sensores[col_temp_sensores], errors="coerce")
df_cetesb[col_temp_cetesb]     = pd.to_numeric(df_cetesb[col_temp_cetesb], errors="coerce")

# Converter Data+Hora para datetime
df_sensores["datetime"] = pd.to_datetime(df_sensores["Data"] + " " + df_sensores["Hora"])

# Corrigir horários 24:00 no CETESB
def corrigir_24h(row):
    if row['Hora'] == "24:00":
        data = pd.to_datetime(row['Data']) + pd.Timedelta(days=1)
        return data.strftime("%Y-%m-%d") + " 00:00"
    else:
        return row['Data'] + " " + row['Hora']

df_cetesb["datetime_str"] = df_cetesb.apply(corrigir_24h, axis=1)
df_cetesb["datetime"] = pd.to_datetime(df_cetesb["datetime_str"])
df_cetesb.drop(columns="datetime_str", inplace=True)

# Arredondar horário dos sensores para hora cheia
df_sensores["datetime"] = df_sensores["datetime"].dt.floor("h")

# Agrupar sensores por hora e tirar média
df_sensores = df_sensores.groupby("datetime", as_index=False)[col_temp_sensores].mean()

# Merge por datetime
df_merge = pd.merge(
    df_sensores,
    df_cetesb,
    on="datetime",
    suffixes=("_sensores", "_cetesb")
)

# Remover NaNs
df_merge = df_merge.dropna(subset=[f"{col_temp_sensores}_sensores", f"{col_temp_cetesb}_cetesb"])

# Calcular métricas
mae  = mean_absolute_error(df_merge[f"{col_temp_cetesb}_cetesb"], df_merge[f"{col_temp_sensores}_sensores"])
rmse = np.sqrt(mean_squared_error(df_merge[f"{col_temp_cetesb}_cetesb"], df_merge[f"{col_temp_sensores}_sensores"]))
mape = np.mean(np.abs((df_merge[f"{col_temp_cetesb}_cetesb"] - df_merge[f"{col_temp_sensores}_sensores"]) / df_merge[f"{col_temp_cetesb}_cetesb"])) * 100
corr = df_merge[f"{col_temp_cetesb}_cetesb"].corr(df_merge[f"{col_temp_sensores}_sensores"])

# Resultados
print("📊 Comparação Temperatura Sensores vs CETESB")
print(f"Quantidade de pontos comparados: {len(df_merge)}")
print(f"MAE  (Erro Absoluto Médio): {mae:.4f} °C")
print(f"RMSE (Raiz do Erro Quadrático Médio): {rmse:.4f} °C")
print(f"MAPE (Erro Percentual Médio): {mape:.2f}%")
print(f"Correlação de Pearson: {corr:.4f}")

# 📈 Gráfico de série temporal
plt.figure(figsize=(12,6))
plt.plot(df_merge["datetime"], df_merge[f"{col_temp_cetesb}_cetesb"], label="CETESB", marker='o')
plt.plot(df_merge["datetime"], df_merge[f"{col_temp_sensores}_sensores"], label="Sensores", marker='x')
plt.xlabel("Data/Hora")
plt.ylabel("Temperatura (°C)")
plt.title("Comparação de Temperatura - Sensores vs CETESB")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("comparacao_temperatura_linhas.png", dpi=300)

# 📉 Gráfico de dispersão (correlação)
plt.figure(figsize=(6,6))
plt.scatter(df_merge[f"{col_temp_cetesb}_cetesb"], df_merge[f"{col_temp_sensores}_sensores"], alpha=0.6)
plt.xlabel("Temperatura CETESB (°C)")
plt.ylabel("Temperatura Sensores (°C)")
plt.title("Dispersão Temperatura - Sensores vs CETESB")
plt.grid(True)
plt.tight_layout()
plt.savefig("comparacao_temperatura_dispersao.png", dpi=300)
