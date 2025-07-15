import pandas as pd
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans
from sklearn.preprocessing import StandardScaler
import logging
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D
import os

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

def main():
    try:
        # Diretórios relativos ao local do script
        BASE_DIR = os.path.dirname(os.path.abspath(__file__))
        data_input_path = os.path.join(BASE_DIR, '../data/processed/sensor_data_cleaned.csv')
        data_output_path = os.path.join(BASE_DIR, '../data/processed/clustered_data_kmeans.csv')
        graphs_output_dir = os.path.join(BASE_DIR, 'graphs')
        os.makedirs(graphs_output_dir, exist_ok=True)

        logging.info(f"Loading dataset from '{data_input_path}'...")
        df = pd.read_csv(data_input_path)
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        X = df[features]

        logging.info("Scaling features...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)

        optimal_k = 3
        logging.info(f"Applying KMeans clustering with K={optimal_k}...")
        kmeans = KMeans(n_clusters=optimal_k, random_state=42, n_init='auto')
        df['Cluster'] = kmeans.fit_predict(X_scaled)
        centroids_unscaled = scaler.inverse_transform(kmeans.cluster_centers_)

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
        plt.title('KMeans: Temperature vs MQ7_CO_PPM')
        plt.colorbar(scatter, label='Cluster')
        plt.legend()
        plt.grid(True)
        plt.savefig(os.path.join(graphs_output_dir, 'kmeans_temp_vs_mq7.png'))
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
        plt.savefig(os.path.join(graphs_output_dir, 'kmeans_humidity_vs_mq4.png'))
        plt.close()

        # --- Pairplot
        logging.info("Generating pairplot...")
        sns.set(style="whitegrid")
        pairplot = sns.pairplot(df[features + ['Cluster']], hue='Cluster', palette='tab10', diag_kind='kde')
        pairplot.fig.suptitle("Pairplot of Features Colored by KMeans Cluster", y=1.02)
        pairplot.savefig(os.path.join(graphs_output_dir, 'kmeans_pairplot.png'))
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
        plt.savefig(os.path.join(graphs_output_dir, 'kmeans_3d_clusters.png'))
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
        plt.savefig(os.path.join(graphs_output_dir, 'kmeans_cluster_means.png'))
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
            plt.savefig(os.path.join(graphs_output_dir, 'kmeans_cluster_over_time.png'))
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
