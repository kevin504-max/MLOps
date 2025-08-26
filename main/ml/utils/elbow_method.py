import pandas as pd
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans
from sklearn.preprocessing import StandardScaler
import logging
import os

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

def main():
    try:
        # Define paths
        BASE_DIR = os.path.dirname(os.path.abspath(__file__))
        data_input_path = os.path.join(BASE_DIR, '../data/processed/sensor_data_cleaned.csv')
        elbow_output_path = os.path.join(BASE_DIR, 'graphs/elbow_plot_simulated.png')
        os.makedirs(os.path.dirname(elbow_output_path), exist_ok=True)

        logging.info(f"Loading CSV file from '{data_input_path}'...")
        df = pd.read_csv(data_input_path)
        logging.info(f"CSV loaded successfully. Total records: {len(df)}")

        # Select relevant features
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        logging.info(f"Using features: {features}")
        X = df[features]

        # Normalize the data
        logging.info("Normalizing features using StandardScaler...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)

        # Elbow Method
        logging.info("Running Elbow Method (K=1 to K=10)...")
        wss = []
        k_range = range(1, 11)
        for k in k_range:
            kmeans = KMeans(n_clusters=k, random_state=42, n_init='auto')
            kmeans.fit(X_scaled)
            wss.append(kmeans.inertia_)
            logging.info(f"K={k}: WSS = {kmeans.inertia_:.2f}")

        # Plot
        logging.info(f"Saving Elbow plot to '{elbow_output_path}'...")
        plt.figure(figsize=(8, 5))
        plt.plot(k_range, wss, 'bo-', label='WSS (inertia)')
        plt.xlabel('Number of Clusters (K)')
        plt.ylabel('WSS (inertia)')
        plt.title('Elbow Method for K-Means Clustering')
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.savefig(elbow_output_path)
        plt.close()

        logging.info("Elbow plot saved successfully.")

    except FileNotFoundError:
        logging.error(f"The file '{data_input_path}' was not found.")
    except Exception as e:
        logging.error(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    main()
