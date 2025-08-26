from datetime import datetime
import pandas as pd
from pathlib import Path
import matplotlib.pyplot as plt

# ---- Caminhos ----
base_path = Path(__file__).resolve().parent.parent / "data" / "processed"
sensor_file = base_path / "sensor_readings_formatado.csv"
output_file = base_path / "sensor_readings_corrigido.csv"

# ---- Ler dados ----
df = pd.read_csv(sensor_file)
df["CO_ppm"] = pd.to_numeric(df["CO_ppm"], errors="coerce")
df["datetime"] = pd.to_datetime(df["Data"] + " " + df["Hora"])
df["hour"] = df["datetime"].dt.hour

# ---- Média histórica do sensor por hora ----
media_historica_horaria = df.groupby("hour")["CO_ppm"].mean()
print("Média histórica por hora:\n", media_historica_horaria)

# ---- Aplicar correção ----
# Ajusta cada leitura para que fique na média histórica da hora correspondente
def corrigir_historico(row):
    media_atual_hora = df[df["hour"] == row["hour"]]["CO_ppm"].mean()
    if media_atual_hora == 0:
        return row["CO_ppm"]
    fator = media_historica_horaria[row["hour"]] / media_atual_hora
    return row["CO_ppm"] * fator

df["CO_ppm_corrigido"] = df.apply(corrigir_historico, axis=1)

# Na prática, isso vai padronizar a leitura para a média horária mantendo a variação relativa

# ---- Salvar CSV corrigido ----
df.to_csv(output_file, index=False)
print(f"CSV corrigido salvo em: {output_file}")

# ---- Gráfico comparativo ----
plt.figure(figsize=(12,6))
plt.plot(df["datetime"], df["CO_ppm"], label="Sensor Original", alpha=0.7)
plt.plot(df["datetime"], df["CO_ppm_corrigido"], label="Sensor Corrigido", alpha=0.7)
plt.xlabel("Data/Hora")
plt.ylabel("CO (ppm)")
plt.title("Correção Baseada em Média Histórica Local")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(base_path / "sensor_readings_corrigido.png", dpi=300)
