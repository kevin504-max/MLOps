# 📡 Sistema de Monitoramento Ambiental com IoT e MLOps

https://img.shields.io/badge/ESP--IDF-v5.1-blue
https://img.shields.io/badge/Python-3.10%252B-blue

Sistema embarcado inteligente para monitoramento da qualidade do ar em tempo real, utilizando microcontrolador ESP32, sensores de baixo custo e técnicas de MLOps para detecção automática de padrões e anomalias.

## 🚀 Instalação e Configuração

### Pré-requisitos

- **Windows 10/11** com WSL2 instalado
- **Ubuntu** ou outra distribuição Linux no WSL
- **ESP-IDF v5.4** instalado no WSL
- **USBIP** para acesso à porta serial do Windows

## 🔧 Configuração do USB no WSL (Windows)

```bash
# No PowerShell do Windows (como Administrador):
# Listar dispositivos USB disponíveis
usbipd list

# Compartilhar a porta do ESP32
usbipd bind --busid <BUSID>

# Conectar ao WSL
usbipd attach --wsl --busid <BUSID>
```

## ⚙️ Configuração do Ambiente de Desenvolvimento

```bash
# No terminal WSL (Ubuntu):

# 1️⃣ Ativar o ambiente do ESP-IDF
. <caminho_projeto_esp>/export.sh

# 2️⃣ Verificar a porta serial do ESP32
ls /dev/ttyUSB*
# Para novas ESP32-S3:
ls -l /dev/ttyACM0

# 3️⃣ Navegar até o diretório do projeto
cd <caminho_seu_diretorio>/MLOps
```

## 🔄 Comandos de Desenvolvimento

```bash
# Compilar o código
idf.py build

# Gravar o firmware no ESP32
idf.py -p /dev/ttyUSB0 flash
# Para ESP32-S3:
idf.py -p /dev/ttyACM0 flash

# Se houver problemas de comunicação, usar baud rate mais baixo:
idf.py -p /dev/ttyUSB0 -b 115200 flash

# Monitorar a saída serial
idf.py -p /dev/ttyUSB0 monitor

# Configurações do projeto (menu interativo)
idf.py menuconfig

# Limpar a memória flash
idf.py -p /dev/ttyUSB0 erase-flash
idf.py flash monitor
```  

## 🚨 Solução de Problemas

```bash
# Matar processo do monitor serial se travar
ps aux | grep idf.py
kill -9 <PID>

# Exemplo: kill -9 12345
```

## 🌐 Acesso aos Dados

```bash
# O ESP32 cria um servidor web após conectar ao Wi-Fi
# Acesse o IP mostrado no monitor serial para download dos dados
http://192.168.15.57/download_csv
# Ou (o IP varia conforme a rede):
http://<ip_da_sua_rede>/download_csv
```

## 🐍 Ambiente Python para Análise

```bash
# No diretório de análise de dados:
python3 -m venv venv
source venv/bin/activate

# Instalar dependências
pip install pandas datetime sklearn numpy matplotlib logging seaborn

# Executar scripts de análise desejado (e.g.):
python algorithms/kmeans_application/kmeans_w_elbow.py
python algorithms/kmeans_application/kmeans_w_siluette.py
```

## 📋 Fluxo de Trabalho Típico

1. **Conectar o ESP32** via USB e compartilhar com WSL

2. **Compilar** o código: `idf.py build`

3. **Gravar** no dispositivo: `idf.py -p /dev/ttyUSB0 flash`

4. **Monitorar** a execução: `idf.py -p /dev/ttyUSB0 monitor`

5. **Coletar** dados via interface web

6. **Analisar** dados com scripts Python

## 🛠️ Troubleshooting Comum

- **Dispositivo não encontrado**: Verifique se o USBIP está corretamente configurado

- **Erro de comunicação**: Tente baud rate mais baixo (`-b 115200`)

- **Problemas de compilação**: Verifique se o ESP-IDF está sincronizado corretamente

- **Falha no flash**: Execute `erase-flash` antes de gravar novamente

## 🌟 Funcionalidades Principais

- ✅ **Aquisição Contínua de Dados**: Medição de temperatura, umidade, metano (CH₄) e monóxido de carbono (CO) em intervalos de 30 minutos

- ✅ **Processamento em Tempo Real**: Pipeline de dados integrado com FreeRTOS para operação concorrente

- ✅ **Detecção Inteligente de Anomalias**: Algoritmo K-Means para identificação automática de padrões de poluição

- ✅ **Interface Web Integrada**: Servidor HTTP para download remoto dos dados em formato CSV

- ✅ **Validação Científica**: Comparação quantitativa com dados da CETESB

## 📊 Resultados Destacados

- **Redução de 34% no MAE e 38%** no **RMSE** após correção por média histórica

- Identificação de **4 clusters ambientais** distintos com Silhouette Score de 0.79

- Sistema validado com **203 pontos de dados** contra padrão CETESB

## 🗂️ Estrutura do Projeto

```bash
📦MLOps
 ┣ 📂.git
 ┣ 📂build
 ┣ 📂components
 ┃ ┣ 📂dht
 ┃ ┣ 📂esp-idf-lib
 ┣ 📂diagrams
 ┃ ┣ 📜package.dio
 ┃ ┣ 📜sequence.dio
 ┃ ┗ 📜uml.dio
 ┣ 📂main
 ┃ ┣ 📂core
 ┃ ┃ ┣ 📂network
 ┃ ┃ ┃ ┣ 📂time_sync
 ┃ ┃ ┃ ┃ ┣ 📜time_sync.c
 ┃ ┃ ┃ ┃ ┗ 📜time_sync.h
 ┃ ┃ ┃ ┣ 📂webserver
 ┃ ┃ ┃ ┃ ┣ 📜webserver.c
 ┃ ┃ ┃ ┃ ┗ 📜webserver.h
 ┃ ┃ ┃ ┗ 📂wifi_connector
 ┃ ┃ ┃ ┃ ┣ 📜wifi_connector.c
 ┃ ┃ ┃ ┃ ┗ 📜wifi_connector.h
 ┃ ┃ ┣ 📂sensors
 ┃ ┃ ┃ ┣ 📂dht
 ┃ ┃ ┃ ┃ ┣ 📜dht_sensor.c
 ┃ ┃ ┃ ┃ ┗ 📜dht_sensor.h
 ┃ ┃ ┃ ┣ 📂mq4
 ┃ ┃ ┃ ┃ ┣ 📜mq4_sensor.c
 ┃ ┃ ┃ ┃ ┗ 📜mq4_sensor.h
 ┃ ┃ ┃ ┗ 📂mq7
 ┃ ┃ ┃ ┃ ┣ 📜mq7_sensor.c
 ┃ ┃ ┃ ┃ ┗ 📜mq7_sensor.h
 ┃ ┃ ┣ 📂shared
 ┃ ┃ ┃ ┣ 📜shared_sensor_data.c
 ┃ ┃ ┃ ┗ 📜shared_sensor_data.h
 ┃ ┃ ┗ 📂storage
 ┃ ┃ ┃ ┣ 📂csv_logger
 ┃ ┃ ┃ ┃ ┣ 📜csv_logger.c
 ┃ ┃ ┃ ┃ ┗ 📜csv_logger.h
 ┃ ┃ ┃ ┣ 📂csv_merger
 ┃ ┃ ┃ ┃ ┣ 📜csv_merger.c
 ┃ ┃ ┃ ┃ ┗ 📜csv_merger.h
 ┃ ┃ ┃ ┣ 📂csv_writer
 ┃ ┃ ┃ ┃ ┣ 📜csv_writer.c
 ┃ ┃ ┃ ┃ ┗ 📜csv_writer.h
 ┃ ┃ ┃ ┗ 📂spiffs_manager
 ┃ ┃ ┃ ┃ ┣ 📜spiffs_manager.c
 ┃ ┃ ┃ ┃ ┗ 📜spiffs_manager.h
 ┃ ┣ 📂helpers
 ┃ ┃ ┗ 📂supervisor
 ┃ ┃ ┃ ┣ 📜supervisor.c
 ┃ ┃ ┃ ┗ 📜supervisor.h
 ┃ ┣ 📂ml
 ┃ ┃ ┣ 📂algorithms
 ┃ ┃ ┃ ┣ 📂dbscan_application
 ┃ ┃ ┃ ┃ ┣ 📂graphs
 ┃ ┃ ┃ ┃ ┗ 📜main.py
 ┃ ┃ ┃ ┣ 📂gmm_application
 ┃ ┃ ┃ ┃ ┣ 📂graphs
 ┃ ┃ ┃ ┃ ┗ 📜main.py
 ┃ ┃ ┃ ┣ 📂kmeans_application
 ┃ ┃ ┃ ┃ ┣ 📂graphs
 ┃ ┃ ┃ ┃ ┣ 📂models
 ┃ ┃ ┃ ┃ ┃ ┣ 📜centroids.npy
 ┃ ┃ ┃ ┃ ┃ ┣ 📜data_stats.csv
 ┃ ┃ ┃ ┃ ┃ ┣ 📜kmeans_model.pkl
 ┃ ┃ ┃ ┃ ┃ ┗ 📜scaler.pkl
 ┃ ┃ ┃ ┃ ┣ 📜kmeans_trainer.py
 ┃ ┃ ┃ ┃ ┣ 📜kmeans_w_elbow.py
 ┃ ┃ ┃ ┃ ┣ 📜kmeans_w_siluette.py
 ┃ ┃ ┃ ┃ ┣ 📜predict_from_input.py
 ┃ ┃ ┃ ┃ ┗ 📜predition.py
 ┃ ┃ ┃ ┗ 📂knn_application
 ┃ ┃ ┃ ┃ ┣ 📂graphs
 ┃ ┃ ┃ ┃ ┃ ┣ 📜knn_classification_report.txt
 ┃ ┃ ┃ ┃ ┃ ┣ 📜knn_classification_report_simulated.txt
 ┃ ┃ ┃ ┃ ┃ ┣ 📜knn_confusion_matrix.png
 ┃ ┃ ┃ ┃ ┃ ┗ 📜knn_confusion_matrix_simulated.png
 ┃ ┃ ┃ ┃ ┗ 📜main.py
 ┃ ┃ ┣ 📂comparison
 ┃ ┃ ┃ ┗ 📂cetesb
 ┃ ┃ ┃ ┃ ┣ 📜cetesb_co.py
 ┃ ┃ ┃ ┃ ┣ 📜cetesb_co_historical.py
 ┃ ┃ ┃ ┃ ┣ 📜cetesb_temp.py
 ┃ ┃ ┃ ┃ ┗ 📜linear_correction_historical.py
 ┃ ┃ ┣ 📂data
 ┃ ┃ ┃ ┣ 📂processed
 ┃ ┃ ┃ ┗ 📂raw
 ┃ ┃ ┃ ┃ ┣ 📂external
 ┃ ┃ ┃ ┃ ┗ 📜merged.csv
 ┃ ┃ ┣ 📂utils
 ┃ ┃ ┃ ┣ 📂cetesb
 ┃ ┃ ┃ ┃ ┣ 📜format_co.py
 ┃ ┃ ┃ ┃ ┗ 📜format_temp.py
 ┃ ┃ ┃ ┣ 📂graphs
 ┃ ┃ ┃ ┃ ┗ 📜elbow_plot_simulated.png
 ┃ ┃ ┃ ┣ 📜elbow_method.py
 ┃ ┃ ┃ ┣ 📜historical_avg_sensor.py
 ┃ ┃ ┃ ┗ 📜sensor_data_clear.py
 ┃ ┣ 📜CMakeLists.txt
 ┃ ┗ 📜main.c
 ┣ 📜CMakeLists.txt
 ┣ 📜README.md
 ┣ 📜build.log
 ┣ 📜partitions.csv
 ┣ 📜sdkconfig
 ┣ 📜sdkconfig.old
 ┗ 📜spiffs_dump.bin
```

# 👥 Autores
**Kevin Lucas de Oliveira Brito** - @kevin504-max