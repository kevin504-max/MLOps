import os
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import classification_report, confusion_matrix
import logging

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

def main():
    try:
        # Caminhos
        BASE_DIR = os.path.dirname(os.path.abspath(__file__))
        data_path = os.path.join(BASE_DIR, '../data/processed/clustered_data_kmeans.csv')
        output_dir = os.path.join(BASE_DIR, 'graphs')
        os.makedirs(output_dir, exist_ok=True)

        logging.info(f"Loading labeled dataset from '{data_path}'...")
        df = pd.read_csv(data_path)

        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        target = 'Cluster'

        if target not in df.columns:
            logging.error("Target 'Cluster' not found in dataset. Run KMeans clustering first.")
            return

        X = df[features]
        y = df[target]

        logging.info("Scaling features...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)

        logging.info("Splitting into train and test sets (80/20)...")
        X_train, X_test, y_train, y_test = train_test_split(X_scaled, y, test_size=0.2, random_state=42)

        k = 5
        logging.info(f"Training KNN classifier with k={k}...")
        knn = KNeighborsClassifier(n_neighbors=k)
        knn.fit(X_train, y_train)

        logging.info("Evaluating model...")
        y_pred = knn.predict(X_test)

        # --- Classification report
        report = classification_report(y_test, y_pred)
        print(report)
        with open(os.path.join(output_dir, 'knn_classification_report.txt'), 'w') as f:
            f.write(report)

        # --- Confusion matrix
        logging.info("Plotting confusion matrix...")
        cm = confusion_matrix(y_test, y_pred)
        sns.heatmap(cm, annot=True, fmt='d', cmap='Blues')
        plt.title('KNN Confusion Matrix')
        plt.xlabel('Predicted')
        plt.ylabel('Actual')
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, 'knn_confusion_matrix.png'))
        plt.close()

        logging.info("KNN training and evaluation completed successfully.")

    except FileNotFoundError:
        logging.error("Labeled dataset file not found. Ensure KMeans has been applied first.")
    except Exception as e:
        logging.error(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
