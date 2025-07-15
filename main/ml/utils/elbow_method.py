import pandas as pd  # For handling tabular data
import matplotlib.pyplot as plt  # For plotting graphs
from sklearn.cluster import KMeans  # K-Means clustering algorithm
from sklearn.preprocessing import StandardScaler  # For data normalization
import logging  # For logging process steps

# Configure logging
logging.basicConfig(
    level=logging.INFO,  # Log level can be adjusted to DEBUG for more details
    format='%(asctime)s - %(levelname)s - %(message)s'
)

def main():
    try:
        logging.info("Starting to load the CSV file 'sensor_data_cleaned.csv'...")
        # 1. Load the dataset
        df = pd.read_csv('../data/processed/sensor_data_cleaned.csv')
        logging.info(f"CSV loaded successfully. Total records: {len(df)}")

        # 2. Select relevant numerical features
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        logging.info(f"Selecting features for clustering: {features}")
        X = df[features]

        # 3. Normalize the features
        logging.info("Applying data normalization using StandardScaler...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)
        logging.info("Normalization completed.")

        # 4. Elbow Method to determine the optimal number of clusters
        logging.info("Starting Elbow Method loop from K=1 to K=10...")
        wss = []  # List to store Within-Cluster Sum of Squares
        k_range = range(1, 11)
        for k in k_range:
            kmeans = KMeans(n_clusters=k, random_state=42, n_init='auto')
            kmeans.fit(X_scaled)
            inertia = kmeans.inertia_
            wss.append(inertia)
            logging.info(f"K={k}: WSS (inertia) = {inertia:.2f}")

        # 5. Plot and save the Elbow Curve
        logging.info("Saving the Elbow Curve as 'elbow_plot.png'...")
        plt.figure(figsize=(8, 5))
        plt.plot(k_range, wss, 'bo-', label='WSS (inertia)')
        plt.xlabel('Number of Clusters (K)')
        plt.ylabel('WSS (inertia)')
        plt.title('Elbow Method for K-Means Clustering')
        plt.grid(True)
        plt.legend()
        plt.savefig("./data/processed/elbow_plot.png")
        plt.close()
        logging.info("Elbow plot saved successfully. Process completed.")

    except FileNotFoundError:
        logging.error("The file 'sensor_data_cleaned.csv' was not found. Please check the file path.")
    except Exception as e:
        logging.error(f"An unexpected error occurred: {e}")

if __name__ == "__main__":
    main()
