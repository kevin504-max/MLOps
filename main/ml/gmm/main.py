import pandas as pd
import matplotlib.pyplot as plt
from sklearn.mixture import GaussianMixture
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
        data_output_path = os.path.join(BASE_DIR, '../data/processed/clustered_data_gmm.csv')
        graphs_output_dir = os.path.join(BASE_DIR, 'graphs')
        os.makedirs(graphs_output_dir, exist_ok=True)

        logging.info(f"Loading dataset from '{data_input_path}'...")
        df = pd.read_csv(data_input_path)
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        X = df[features]

        logging.info("Scaling features...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)

        n_components = 4  # Ajuste conforme o seu elbow/aic/bic
        logging.info(f"Applying Gaussian Mixture Model clustering with n_components={n_components}...")
        gmm = GaussianMixture(n_components=n_components, random_state=42)
        clusters = gmm.fit_predict(X_scaled)
        df['Cluster'] = clusters

        # Centroids aproximados pelo GMM (media das componentes)
        centroids_unscaled = scaler.inverse_transform(gmm.means_)

        df.to_csv(data_output_path, index=False)
        logging.info(f"Clustered data saved to '{data_output_path}'.")

        # Mapear clusters para labels descritivos (exemplo simples)
        cluster_labels_map = {}
        for i, center in enumerate(centroids_unscaled): 
            mq4 = center[2]
            mq7 = center[3]

            print (f"MQ4: {mq4}, MQ7: {mq7}")
            if mq4 < 3 and mq7 < 1:
                label = 'Ar limpo'
            elif mq4 < 6 and mq7 < 3:
                label = 'Moderadamente poluído'
            else:
                label = 'Altamente poluído'
            cluster_labels_map[i] = label
        df['Cluster_Label'] = df['Cluster'].map(cluster_labels_map)

        # --- Plot 1: Temperature vs MQ7_CO_PPM
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(df['Temperature(C)'], df['MQ7_CO_PPM'], c=df['Cluster'], cmap='viridis', alpha=0.7)
        plt.scatter(
            centroids_unscaled[:, 0], centroids_unscaled[:, 3],
            c='red', marker='X', s=200, edgecolors='black', label='Centroids'
        )
        plt.xlabel('Temperature (C)')
        plt.ylabel('MQ7 CO PPM')
        plt.title('GMM Clusters: Temperature vs MQ7_CO_PPM')
        cbar = plt.colorbar(scatter)
        cbar.set_label('Cluster')
        cbar.set_ticks(sorted(df['Cluster'].unique()))
        cbar.set_ticklabels([cluster_labels_map[i] for i in sorted(df['Cluster'].unique())])
        plt.legend()
        plt.grid(True)
        plt.savefig(os.path.join(graphs_output_dir, 'gmm_temp_vs_mq7.png'))
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
        plt.title('GMM Clusters: Humidity vs MQ4_PPM')
        cbar = plt.colorbar(scatter)
        cbar.set_label('Cluster')
        cbar.set_ticks(sorted(df['Cluster'].unique()))
        cbar.set_ticklabels([cluster_labels_map[i] for i in sorted(df['Cluster'].unique())])
        plt.legend()
        plt.grid(True)
        plt.savefig(os.path.join(graphs_output_dir, 'gmm_humidity_vs_mq4.png'))
        plt.close()

        # --- Pairplot
        logging.info("Generating pairplot...")
        sns.set(style="whitegrid")
        pairplot = sns.pairplot(df[features + ['Cluster_Label']], hue='Cluster_Label', palette='tab10', diag_kind='kde')
        pairplot.fig.suptitle("Pairplot of Features Colored by GMM Cluster", y=1.02)
        pairplot.savefig(os.path.join(graphs_output_dir, 'gmm_pairplot.png'))
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
        ax.set_title('3D Clustering Visualization (GMM)')
        plt.savefig(os.path.join(graphs_output_dir, 'gmm_3d_clusters.png'))
        plt.close()

        # --- Cluster Means
        logging.info("Plotting cluster centroid averages...")
        centroid_df = pd.DataFrame(centroids_unscaled, columns=features)
        centroid_df['Cluster'] = range(n_components)
        centroid_df['Label'] = centroid_df['Cluster'].map(cluster_labels_map)
        centroid_df.set_index('Cluster', inplace=True)
        centroid_df.plot(kind='bar', figsize=(10, 6))
        plt.title('Average Sensor Readings per GMM Cluster')
        plt.xlabel('Cluster')
        plt.ylabel('Original Value')
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(os.path.join(graphs_output_dir, 'gmm_cluster_means.png'))
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
            plt.title('GMM Cluster Assignment Over Time')
            plt.grid(True)
            plt.tight_layout()
            plt.savefig(os.path.join(graphs_output_dir, 'gmm_cluster_over_time.png'))
            plt.close()
        else:
            logging.warning("Column 'Timestamp' not found. Skipping time series plot.")

        logging.info("All GMM visualizations saved successfully.")

    except FileNotFoundError:
        logging.error(f"The file '{data_input_path}' was not found.")
    except Exception as e:
        logging.error(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
