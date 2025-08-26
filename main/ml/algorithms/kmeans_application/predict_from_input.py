import os
import joblib
import numpy as np
import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

base_dir = os.path.dirname(os.path.abspath(__file__))
model_path = os.path.join(base_dir, 'models', 'kmeans_model.pkl')
scaler_path = os.path.join(base_dir, 'models', 'scaler.pkl')

# Carrega modelo e scaler
kmeans = joblib.load(model_path)
scaler = joblib.load(scaler_path)

def predict_cluster(data_point):
    """
    data_point: dict com keys = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
    """
    features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
    X = np.array([[data_point[feat] for feat in features]])
    X_scaled = scaler.transform(X)
    cluster = kmeans.predict(X_scaled)[0]
    return cluster

def generate_alert(cluster):
    """
    Exemplo simples de alerta baseado no cluster.
    Ajuste conforme lógica do seu projeto.
    """
    # Exemplo: cluster 3 é o crítico
    if cluster == 3:
        return "Alerta: Condição crítica detectada no cluster 3!"
    else:
        return "Condição normal."

if __name__ == "__main__":
    # Exemplo de dados de entrada
    sample_data = [
        {'Temperature(C)': 22.3, 'Humidity(%)': 52.0, 'MQ4_PPM': 5.1,  'MQ7_CO_PPM': 0.29},
        {'Temperature(C)': 26.6, 'Humidity(%)': 39.2, 'MQ4_PPM': 122.0,'MQ7_CO_PPM': 2.6},
        {'Temperature(C)': 21.1, 'Humidity(%)': 74.5, 'MQ4_PPM': 8.0,  'MQ7_CO_PPM': 0.6},
        {'Temperature(C)': 23.4, 'Humidity(%)': 55.3, 'MQ4_PPM': 6.0, 'MQ7_CO_PPM': 15.1},
    ]

    for data in sample_data:
        cluster_pred = predict_cluster(data)
        alert_msg = generate_alert(cluster_pred)
        logging.info(f"Entrada: {data} => Cluster previsto: {cluster_pred} => {alert_msg}")
