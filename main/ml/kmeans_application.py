import pandas as pd
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans
from sklearn.preprocessing import StandardScaler
import logging
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

def main():
    try:
        logging.info("Loading dataset from 'merged.csv'...")
        df = pd.read_csv('merged.csv')
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        X = df[features]

        logging.info("Scaling features...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)

        optimal_k = 4  # ← change this based on your elbow plot
        logging.info(f"Applying KMeans clustering with K={optimal_k}...")
        kmeans = KMeans(n_clusters=optimal_k, random_state=42, n_init='auto')
        df['Cluster'] = kmeans.fit_predict(X_scaled)
        centroids = kmeans.cluster_centers_

        # Save CSV
        df.to_csv('clustered_data.csv', index=False)
        logging.info("Clustered data saved to 'clustered_data.csv'.")

        # --- Visualization 1: 2D Scatter Plot (Temperature vs MQ7_CO_PPM)
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(
            df['Temperature(C)'], df['MQ7_CO_PPM'],
            c=df['Cluster'], cmap='viridis', alpha=0.7
        )
        plt.xlabel('Temperature (C)')
        plt.ylabel('MQ7 CO PPM')
        plt.title('Clusters: Temperature vs MQ7_CO_PPM')
        plt.colorbar(scatter, label='Cluster')
        plt.grid(True)
        plt.savefig('plot_temp_vs_mq7.png')
        plt.close()

        # --- Visualization 2: 2D Scatter Plot (Humidity vs MQ4_PPM)
        plt.figure(figsize=(8, 6))
        scatter = plt.scatter(
            df['Humidity(%)'], df['MQ4_PPM'],
            c=df['Cluster'], cmap='plasma', alpha=0.7
        )
        plt.xlabel('Humidity (%)')
        plt.ylabel('MQ4 PPM')
        plt.title('Clusters: Humidity vs MQ4_PPM')
        plt.colorbar(scatter, label='Cluster')
        plt.grid(True)
        plt.savefig('plot_humidity_vs_mq4.png')
        plt.close()

        # --- Visualization 3: Seaborn Pairplot
        logging.info("Generating pairplot (matriz de dispersão)...")
        sns.set(style="whitegrid")
        pairplot = sns.pairplot(df[features + ['Cluster']], hue='Cluster', palette='tab10', diag_kind='kde')
        pairplot.fig.suptitle("Pairplot of Features Colored by Cluster", y=1.02)
        pairplot.savefig('pairplot_clusters.png')
        plt.close()

        # --- Visualization 4: 3D Scatter Plot
        logging.info("Generating 3D scatter plot...")
        fig = plt.figure(figsize=(10, 7))
        ax = fig.add_subplot(111, projection='3d')
        ax.scatter(
            df['Temperature(C)'], df['Humidity(%)'], df['MQ7_CO_PPM'],
            c=df['Cluster'], cmap='Spectral', alpha=0.8
        )
        ax.set_xlabel('Temperature (C)')
        ax.set_ylabel('Humidity (%)')
        ax.set_zlabel('MQ7 CO PPM')
        ax.set_title('3D Clustering Visualization')
        plt.savefig('plot_3d_clusters.png')
        plt.close()

        # --- Visualization 5: Cluster Means (Centroids)
        logging.info("Plotting cluster centroid averages...")
        centroid_df = pd.DataFrame(scaler.inverse_transform(centroids), columns=features)
        centroid_df['Cluster'] = range(optimal_k)
        centroid_df.set_index('Cluster', inplace=True)
        centroid_df.plot(kind='bar', figsize=(10, 6))
        plt.title('Average Sensor Readings per Cluster (Centroids)')
        plt.xlabel('Cluster')
        plt.ylabel('Original Value')
        plt.grid(True)
        plt.tight_layout()
        plt.savefig('plot_centroid_means.png')
        plt.close()

        logging.info("All visualizations saved successfully.")

    except FileNotFoundError:
        logging.error("The file 'merged.csv' was not found.")
    except Exception as e:
        logging.error(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
