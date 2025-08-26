from pathlib import Path
import csv
from datetime import datetime

# Caminho relativo à pasta onde está o script utils/
entrada = Path(__file__).parent.parent / "data" / "raw" / "cetesb_co_congonhas.csv"
saida = Path(__file__).parent.parent / "data" / "processed" / "cetesb_co_congonhas_formatado.csv"

with open(entrada, "r", encoding="latin1") as f:
    linhas = f.readlines()

dados_iniciados = False
linhas_dados = []
for linha in linhas:
    if linha.strip().startswith("Data;Hora"):
        dados_iniciados = True
        continue
    if dados_iniciados:
        if linha.strip().startswith(";;"):
            continue
        linhas_dados.append(linha.strip())

with open(saida, "w", newline="", encoding="utf-8") as f_out:
    writer = csv.writer(f_out)
    writer.writerow(["Data", "Hora", "CO_ppm"])
    for linha in linhas_dados:
        if not linha.strip():
            continue
        partes = linha.split(";")
        if len(partes) >= 3:
            try:
                data_formatada = datetime.strptime(partes[0], "%d/%m/%Y").strftime("%Y-%m-%d")
            except ValueError:
                data_formatada = partes[0]
            hora = partes[1]
            valor = partes[2].replace(",", ".") if partes[2] else ""
            writer.writerow([data_formatada, hora, valor])

print(f"Arquivo convertido salvo em: {saida}")
