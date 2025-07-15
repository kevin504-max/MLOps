import pandas as pd
import matplotlib.pyplot as plt
from sklearn.cluster import DBSCAN
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
        BASE_DIR = os.path.dirname(os.path.abspath(__file__))
        data_input_path = os.path.join(BASE_DIR, '../data/processed/sensor_data_cleaned.csv')
        data_output_path = os.path.join(BASE_DIR, '../data/processed/clustered_data_dbscan.csv')
        graphs_output_dir = os.path.join(BASE_DIR, 'graphs')

        os.makedirs(graphs_output_dir, exist_ok=True)

        logging.info(f"Loading dataset from '{data_input_path}'...")
        df = pd.read_csv(data_input_path)
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        X = df[features]

        logging.info("Scaling features...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)

        # --- DBSCAN
        eps = 0.6
        min_samples = 5
        logging.info(f"Applying DBSCAN clustering (eps={eps}, min_samples={min_samples})...")
        dbscan = DBSCAN(eps=eps, min_samples=min_samples)
        df['Cluster'] = dbscan.fit_predict(X_scaled)

        num_clusters = len(set(df['Cluster'])) - (1 if -1 in df['Cluster'].values else 0)
        num_noise = sum(df['Cluster'] == -1)
        logging.info(f"DBSCAN identified {num_clusters} clusters and {num_noise} noise points.")

        df.to_csv(data_output_path, index=False)
        logging.info(f"Clustered data saved to '{data_output_path}'.")

        # --- Plot 1
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(df['Temperature(C)'], df['MQ7_CO_PPM'], c=df['Cluster'], cmap='viridis', alpha=0.7)
        plt.xlabel('Temperature (C)')
        plt.ylabel('MQ7 CO PPM')
        plt.title('DBSCAN Clusters: Temperature vs MQ7_CO_PPM')
        plt.colorbar(scatter, label='Cluster')
        plt.grid(True)
        plt.savefig(os.path.join(graphs_output_dir, 'dbscan_temp_vs_mq7.png'))
        plt.close()

        # --- Plot 2
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(df['Humidity(%)'], df['MQ4_PPM'], c=df['Cluster'], cmap='plasma', alpha=0.7)
        plt.xlabel('Humidity (%)')
        plt.ylabel('MQ4 PPM')
        plt.title('DBSCAN Clusters: Humidity vs MQ4_PPM')
        plt.colorbar(scatter, label='Cluster')
        plt.grid(True)
        plt.savefig(os.path.join(graphs_output_dir, 'dbscan_humidity_vs_mq4.png'))
        plt.close()

        # --- Pairplot
        logging.info("Generating pairplot...")
        sns.set(style="whitegrid")
        pairplot = sns.pairplot(df[features + ['Cluster']], hue='Cluster', palette='tab10', diag_kind='kde')
        pairplot.fig.suptitle("Pairplot of Features Colored by DBSCAN Cluster", y=1.02)
        pairplot.savefig(os.path.join(graphs_output_dir, 'dbscan_pairplot.png'))
        plt.close()

        # --- 3D Scatter
        logging.info("Generating 3D scatter plot...")
        fig = plt.figure(figsize=(10, 7))
        ax = fig.add_subplot(111, projection='3d')
        ax.scatter(df['Temperature(C)'], df['Humidity(%)'], df['MQ7_CO_PPM'], c=df['Cluster'], cmap='Spectral', alpha=0.8)
        ax.set_xlabel('Temperature (C)')
        ax.set_ylabel('Humidity (%)')
        ax.set_zlabel('MQ7 CO PPM')
        ax.set_title('3D Visualization of DBSCAN Clusters')
        plt.savefig(os.path.join(graphs_output_dir, 'dbscan_3d_clusters.png'))
        plt.close()

        # --- Cluster Means
        logging.info("Plotting cluster means...")
        df_valid = df[df['Cluster'] != -1]
        if not df_valid.empty:
            cluster_means = df_valid.groupby('Cluster')[features].mean()
            cluster_means.plot(kind='bar', figsize=(10, 6))
            plt.title('Average Sensor Readings per DBSCAN Cluster')
            plt.xlabel('Cluster')
            plt.ylabel('Original Value')
            plt.grid(True)
            plt.tight_layout()
            plt.savefig(os.path.join(graphs_output_dir, 'dbscan_cluster_means.png'))
            plt.close()
        else:
            logging.warning("Only noise detected — no valid clusters to plot.")

        # --- Cluster vs Timestamp
        if 'Timestamp' in df.columns:
            logging.info("Plotting Timestamp vs Cluster visualization...")
            df['Timestamp'] = pd.to_datetime(df['Timestamp'], errors='coerce')
            df = df.dropna(subset=['Timestamp'])
            df = df.sort_values('Timestamp')
            plt.figure(figsize=(12, 6))
            plt.plot(df['Timestamp'], df['Cluster'], marker='o', linestyle='-', alpha=0.6)
            plt.xlabel('Timestamp')
            plt.ylabel('Cluster')
            plt.title('DBSCAN Cluster Assignment Over Time')
            plt.grid(True)
            plt.tight_layout()
            plt.savefig(os.path.join(graphs_output_dir, 'dbscan_cluster_over_time.png'))
            plt.close()
        else:
            logging.warning("Column 'Timestamp' not found. Skipping time series plot.")

        logging.info("All DBSCAN visualizations saved successfully.")

    except FileNotFoundError:
        logging.error(f"The file '{data_input_path}' was not found.")
    except Exception as e:
        logging.error(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
