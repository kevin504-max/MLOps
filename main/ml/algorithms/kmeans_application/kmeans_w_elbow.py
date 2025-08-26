import pandas as pd
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans
from sklearn.preprocessing import StandardScaler
import logging
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D
import os
from sklearn.metrics import silhouette_score

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

def evaluate_clusters(X_scaled, optimal_k):
    """Função separada para avaliação dos clusters"""
    kmeans = KMeans(n_clusters=optimal_k, random_state=42, n_init='auto')
    cluster_labels = kmeans.fit_predict(X_scaled)
    
    # Cálculo do silhouette score
    silhouette_avg = silhouette_score(X_scaled, cluster_labels)
    
    # Interpretação mais detalhada
    interpretation = ""
    if silhouette_avg > 0.7:
        interpretation = "Estrutura de clusters forte."
    elif silhouette_avg > 0.5:
        interpretation = "Estrutura de clusters razoável."
    elif silhouette_avg > 0.25:
        interpretation = "Estrutura de clusters fraca ou artificial."
    else:
        interpretation = "Nenhuma estrutura substancial encontrada."
    
    return kmeans, cluster_labels, silhouette_avg, interpretation

def main():
    try:
        # Paths relative to the script location
        BASE_DIR = os.path.dirname(os.path.abspath(__file__))
        data_input_path = os.path.join(BASE_DIR, '../../data/processed/sensor_data_cleaned.csv')
        data_output_path = os.path.join(BASE_DIR, '../../data/processed/clustered_data_kmeans.csv')
        metrics_output_path = os.path.join(BASE_DIR, '../../data/processed/clustering_metrics.csv')  # Novo arquivo para métricas
        graphs_output_dir = os.path.join(BASE_DIR, 'graphs')
        os.makedirs(graphs_output_dir, exist_ok=True)

        logging.info(f"Loading dataset from '{data_input_path}'...")
        df = pd.read_csv(data_input_path)
        
        # Verificação de dados faltantes
        if df.isnull().values.any():
            logging.warning("Dados contêm valores faltantes. Considerar tratamento.")
        
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        X = df[features]

        logging.info("Scaling features...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)

        optimal_k = 4
        logging.info(f"Applying KMeans clustering with K={optimal_k}...")
        
        # Avaliação dos clusters (função separada)
        kmeans, cluster_labels, silhouette_avg, interpretation = evaluate_clusters(X_scaled, optimal_k)
        
        logging.info(f"Silhouette Score: {silhouette_avg:.3f}")
        logging.info(interpretation)
        
        # Adiciona clusters ao DataFrame
        df['Cluster'] = cluster_labels
        centroids_unscaled = scaler.inverse_transform(kmeans.cluster_centers_)

        # --- Cluster Centroids (valores originais)
        centroid_df = pd.DataFrame(
            centroids_unscaled, 
            columns=features
        )
        centroid_df['Cluster'] = range(optimal_k)
        centroid_df.set_index('Cluster', inplace=True)

        logging.info("Centroid values for each cluster (original scale):")
        logging.info(f"\n{centroid_df}")

        # Salva os centróides em CSV
        centroids_output_path = os.path.join(BASE_DIR, '../data/processed/cluster_centroids.csv')
        centroid_df.to_csv(centroids_output_path)
        logging.info(f"Cluster centroids saved to '{centroids_output_path}'")

        # Salva métricas em arquivo separado
        metrics_df = pd.DataFrame({
            'silhouette_score': [silhouette_avg],
            'n_clusters': [optimal_k],
            'inertia': [kmeans.inertia_]
        })
        metrics_df.to_csv(metrics_output_path, index=False)
        logging.info(f"Cluster metrics saved to '{metrics_output_path}'")

        # [Restante do seu código de visualização permanece igual...]
        df.to_csv(data_output_path, index=False)
        logging.info(f"Clustered data saved to '{data_output_path}'.")

        # --- Plot 1: Temperature vs MQ7_CO_PPM
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(df['Temperature(C)'], df['MQ7_CO_PPM'], c=df['Cluster'], cmap='viridis', alpha=0.7)
        plt.scatter(
            centroids_unscaled[:, 0], centroids_unscaled[:, 3],
            c='red', marker='X', s=200, edgecolors='black', label='Centroids'
        )
        plt.xlabel('Temperature (C)')
        plt.ylabel('MQ7 CO PPM')
        plt.title(f'KMeans: Temperature vs MQ7_CO_PPM\nSilhouette Score: {silhouette_avg:.2f}')
        plt.colorbar(scatter, label='Cluster')
        plt.legend()
        plt.grid(True)
        plt.savefig(os.path.join(graphs_output_dir, 'kmeans_temp_vs_mq7_simulated.png'))
        plt.close()

        # --- Plot 2: Humidity vs MQ4_PPM
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(df['Humidity(%)'], df['MQ4_PPM'], c=df['Cluster'], cmap='plasma', alpha=0.7)
        plt.scatter(
            centroids_unscaled[:, 1], centroids_unscaled[:, 2],
            c='black', marker='X', s=200, edgecolors='white', label='Centroids'
        )
        plt.xlabel('Humidity (%)')
        plt.ylabel('MQ4 PPM')
        plt.title('KMeans: Humidity vs MQ4_PPM')
        plt.colorbar(scatter, label='Cluster')
        plt.legend()
        plt.grid(True)
        plt.savefig(os.path.join(graphs_output_dir, 'kmeans_humidity_vs_mq4_simulated.png'))
        plt.close()

        # --- Pairplot
        logging.info("Generating pairplot...")
        sns.set(style="whitegrid")
        pairplot = sns.pairplot(df[features + ['Cluster']], hue='Cluster', palette='tab10', diag_kind='kde')
        pairplot.fig.suptitle("Pairplot of Features Colored by KMeans Cluster", y=1.02)
        pairplot.savefig(os.path.join(graphs_output_dir, 'kmeans_pairplot_simulated.png'))
        plt.close()

        # --- 3D Scatter Plot
        logging.info("Generating 3D scatter plot...")
        fig = plt.figure(figsize=(10, 7))
        ax = fig.add_subplot(111, projection='3d')
        ax.scatter(df['Temperature(C)'], df['Humidity(%)'], df['MQ7_CO_PPM'],
                   c=df['Cluster'], cmap='Spectral', alpha=0.8)
        ax.set_xlabel('Temperature (C)')
        ax.set_ylabel('Humidity (%)')
        ax.set_zlabel('MQ7 CO PPM')
        ax.set_title('3D Clustering Visualization (KMeans)')
        plt.savefig(os.path.join(graphs_output_dir, 'kmeans_3d_clusters_simulated.png'))
        plt.close()

        # --- Cluster Means
        logging.info("Plotting cluster centroid averages...")
        centroid_df = pd.DataFrame(centroids_unscaled, columns=features)
        centroid_df['Cluster'] = range(optimal_k)
        centroid_df.set_index('Cluster', inplace=True)
        centroid_df.plot(kind='bar', figsize=(10, 6))
        plt.title('Average Sensor Readings per Cluster (Centroids)')
        plt.xlabel('Cluster')
        plt.ylabel('Original Value')
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(os.path.join(graphs_output_dir, 'kmeans_cluster_means_simulated.png'))
        plt.close()

        # --- Cluster vs Timestamp
        if 'Timestamp' in df.columns:
            logging.info("Plotting Timestamp vs Cluster visualization...")
            df['Timestamp'] = pd.to_datetime(df['Timestamp'], errors='coerce')
            df = df.dropna(subset=['Timestamp']).sort_values('Timestamp')

            plt.figure(figsize=(12, 6))
            plt.plot(df['Timestamp'], df['Cluster'], marker='o', linestyle='-', alpha=0.6)
            plt.xlabel('Timestamp')
            plt.ylabel('Cluster')
            plt.title('KMeans Cluster Assignment Over Time')
            plt.grid(True)
            plt.tight_layout()
            plt.savefig(os.path.join(graphs_output_dir, 'kmeans_cluster_over_time_simulated.png'))
            plt.close()
        else:
            logging.warning("Column 'Timestamp' not found. Skipping time series plot.")

        logging.info("All KMeans visualizations saved successfully.")

    except FileNotFoundError:
        logging.error(f"The file '{data_input_path}' was not found.")
    except Exception as e:
        logging.error(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
