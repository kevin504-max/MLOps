import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
import logging
from sklearn.cluster import KMeans
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import silhouette_score
from sklearn.linear_model import LinearRegression
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D

# --- Logging setup ---
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

# --- Função para calcular tendência temporal ---
def calc_trend(series, window=10):
    """
    Calcula a inclinação (slope) da tendência linear
    em uma janela móvel usando regressão linear.
    """
    slopes = []
    X_idx = np.arange(window).reshape(-1, 1)
    for i in range(len(series)):
        if i < window:
            slopes.append(0)
        else:
            y_window = series[i-window:i].values.reshape(-1, 1)
            model = LinearRegression()
            model.fit(X_idx, y_window)
            slopes.append(model.coef_[0][0])  # slope
    return pd.Series(slopes, index=series.index)

# --- Função para encontrar o melhor K ---
def find_optimal_k(X_scaled, k_min=2, k_max=10):
    best_k = k_min
    best_score = -1
    scores = []
    for k in range(k_min, k_max+1):
        kmeans = KMeans(n_clusters=k, random_state=42, n_init='auto')
        labels = kmeans.fit_predict(X_scaled)
        score = silhouette_score(X_scaled, labels)
        scores.append(score)
        if score > best_score:
            best_score = score
            best_k = k
    return best_k, scores

# --- Função principal ---
def main():
    try:
        BASE_DIR = os.path.dirname(os.path.abspath(__file__))
        data_input_path = os.path.join(BASE_DIR, '../../data/processed/sensor_data_cleaned.csv')
        graphs_output_dir = os.path.join(BASE_DIR, 'graphs')
        os.makedirs(graphs_output_dir, exist_ok=True)

        logging.info(f"Loading dataset from '{data_input_path}'...")
        df = pd.read_csv(data_input_path)

        if df.isnull().values.any():
            logging.warning("Dados contêm valores faltantes. Considerar tratamento.")

        # --- Features originais ---
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']

        # --- Feature de tendência temporal ---
        df['Temp_trend'] = calc_trend(df['Temperature(C)'], window=10)
        df['Humidity_trend'] = calc_trend(df['Humidity(%)'], window=10)

        # --- Feature de log-transform para suavizar escala ---
        df['MQ4_log'] = np.log1p(df['MQ4_PPM'])
        df['MQ7_log'] = np.log1p(df['MQ7_CO_PPM'])

        # --- Seleciona features para clusterização ---
        clustering_features = features + ['Temp_trend', 'Humidity_trend', 'MQ4_log', 'MQ7_log']

        logging.info(f"Selected features for clustering: {clustering_features}")

        # --- Escalonamento ---
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(df[clustering_features])

        # --- Encontrar melhor k ---
        logging.info("Finding optimal k using silhouette score...")
        optimal_k, silhouette_scores = find_optimal_k(X_scaled, k_min=2, k_max=10)
        logging.info(f"Optimal k found: {optimal_k} with silhouette score {max(silhouette_scores):.3f}")

        # --- Gráfico do silhouette score por k ---
        plt.figure(figsize=(8, 5))
        plt.plot(range(2, 11), silhouette_scores, marker='o')
        plt.title("Silhouette Score por número de clusters (k)")
        plt.xlabel("Número de clusters (k)")
        plt.ylabel("Silhouette Score")
        plt.grid(True)
        plt.savefig(os.path.join(graphs_output_dir, 'silhouette_scores.png'))
        plt.close()

        # --- Rodar KMeans com melhor k ---
        kmeans = KMeans(n_clusters=optimal_k, random_state=42, n_init='auto')
        df['Cluster'] = kmeans.fit_predict(X_scaled)
        centroids_unscaled = scaler.inverse_transform(kmeans.cluster_centers_)

        # --- Visualizações ---
        sns.set(style="whitegrid")

        # Scatter 2D
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(df['Temperature(C)'], df['MQ7_CO_PPM'],
                              c=df['Cluster'], cmap='viridis', alpha=0.7)
        plt.scatter(centroids_unscaled[:, 0], centroids_unscaled[:, 3],
                    c='red', marker='X', s=200, edgecolors='black', label='Centroids')
        plt.xlabel('Temperature (C)')
        plt.ylabel('MQ7 CO PPM')
        plt.title('Clusters (Temperature vs MQ7_CO_PPM)')
        plt.legend()
        plt.grid(True)
        plt.colorbar(scatter)
        plt.savefig(os.path.join(graphs_output_dir, 'clusters_temp_vs_mq7.png'))
        plt.close()

        # Scatter 2D
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(df['Humidity(%)'], df['MQ4_PPM'],
                              c=df['Cluster'], cmap='viridis', alpha=0.7)
        plt.scatter(centroids_unscaled[:, 1], centroids_unscaled[:, 2],
                    c='red', marker='X', s=200, edgecolors='black', label='Centroids')
        plt.xlabel('Humidity (%)')
        plt.ylabel('MQ4 PPM')
        plt.title('Clusters (Humidity vs MQ4_PPM)')
        plt.legend()
        plt.grid(True)
        plt.colorbar(scatter)
        plt.savefig(os.path.join(graphs_output_dir, 'clusters_humidity_vs_mq4.png'))
        plt.close()

        # Pairplot
        pairplot = sns.pairplot(df[features + ['Cluster']], hue='Cluster', palette='tab10')
        pairplot.fig.suptitle("Pairplot por Cluster", y=1.02)
        pairplot.savefig(os.path.join(graphs_output_dir, 'pairplot_clusters.png'))
        plt.close()

        # 3D Scatter
        fig = plt.figure(figsize=(10, 7))
        ax = fig.add_subplot(111, projection='3d')
        ax.scatter(df['Temperature(C)'], df['Humidity(%)'], df['MQ7_CO_PPM'],
                   c=df['Cluster'], cmap='Spectral', alpha=0.8)
        ax.set_xlabel('Temperature (C)')
        ax.set_ylabel('Humidity (%)')
        ax.set_zlabel('MQ7 CO PPM')
        ax.set_title('Visualização 3D - KMeans')
        plt.savefig(os.path.join(graphs_output_dir, 'clusters_3d.png'))
        plt.close()

        logging.info("Clustering and visualizations completed successfully.")

    except FileNotFoundError:
        logging.error(f"The file '{data_input_path}' was not found.")
    except Exception as e:
        logging.error(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
