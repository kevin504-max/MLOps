import pandas as pd
import os
import logging
from sklearn.cluster import KMeans
from sklearn.preprocessing import StandardScaler
import joblib
import numpy as np

# Configuração de logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def calculate_basic_stats(df, features):
    stats = {}
    stats['mean'] = df[features].mean().to_dict()
    stats['var'] = df[features].var().to_dict()
    stats['q1'] = df[features].quantile(0.25).to_dict()
    stats['q3'] = df[features].quantile(0.75).to_dict()
    return stats

def compare_stats(stats_old, stats_new, threshold=0.1):
    """
    Compara estatísticas: retorna True se houver diferença significativa (>threshold relativo)
    threshold é proporção relativa da diferença permitida, ex: 0.1 = 10%
    """
    for key in stats_old.keys():
        for feat in stats_old[key].keys():
            old_val = stats_old[key][feat]
            new_val = stats_new[key][feat]
            if old_val == 0:
                # evita divisão por zero, se antigo zero, compara diferença absoluta
                diff = abs(new_val - old_val)
                if diff > threshold:
                    logging.info(f"Diferença significativa em {key} para {feat}: {old_val} vs {new_val}")
                    return True
            else:
                rel_diff = abs(new_val - old_val) / abs(old_val)
                if rel_diff > threshold:
                    logging.info(f"Diferença significativa em {key} para {feat}: {old_val} vs {new_val} (rel {rel_diff:.2f})")
                    return True
    return False

def compare_centroids(old_centroids, new_centroids, dist_threshold=0.2):
    """
    Compara centróides antigos e novos pelo máximo da distância euclidiana entre centróides correspondentes.
    Retorna True se alguma distância for maior que dist_threshold.
    """
    from scipy.spatial.distance import cdist

    distances = np.linalg.norm(old_centroids - new_centroids, axis=1)
    max_dist = distances.max()
    if max_dist > dist_threshold:
        logging.info(f"Distância máxima entre centróides mudou significativamente: {max_dist:.3f}")
        return True
    return False

def save_stats(stats, path):
    df = pd.DataFrame(stats)
    df.to_csv(path, index=True)

def load_stats(path):
    if os.path.exists(path):
        return pd.read_csv(path, index_col=0).to_dict()
    else:
        return None

def main():
    try:
        # Caminhos
        BASE_DIR = os.path.dirname(os.path.abspath(__file__))
        input_csv_path = os.path.join(BASE_DIR, '../data/processed/sensor_data_cleaned.csv')
        output_csv_path = os.path.join(BASE_DIR, '../data/processed/clustered_data_kmeans_trained.csv')
        model_output_path = os.path.join(BASE_DIR, 'models/kmeans_model.pkl')
        scaler_output_path = os.path.join(BASE_DIR, 'models/scaler.pkl')
        stats_path = os.path.join(BASE_DIR, 'models/data_stats.csv')
        centroids_path = os.path.join(BASE_DIR, 'models/centroids.npy')
        os.makedirs(os.path.dirname(model_output_path), exist_ok=True)

        # Carregar dados
        logging.info(f"Lendo dados de '{input_csv_path}'...")
        df = pd.read_csv(input_csv_path)

        # Features usadas para clusterização
        features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
        X = df[features]

        # Escalando os dados
        logging.info("Aplicando StandardScaler...")
        scaler = StandardScaler()
        X_scaled = scaler.fit_transform(X)

        # Calcular estatísticas do novo dataset
        stats_new = calculate_basic_stats(df, features)

        # Carregar estatísticas antigas para comparação
        if os.path.exists(stats_path):
            logging.info("Carregando estatísticas anteriores para comparação...")
            stats_old_df = pd.read_csv(stats_path, index_col=0)
            stats_old = stats_old_df.to_dict()
            # Ajusta formato para compare_stats
            stats_old_reformat = {
                'mean': {k: float(stats_old[k]['mean']) for k in features},
                'var': {k: float(stats_old[k]['var']) for k in features},
                'q1': {k: float(stats_old[k]['q1']) for k in features},
                'q3': {k: float(stats_old[k]['q3']) for k in features},
            }
            diff_data = compare_stats(stats_old_reformat, stats_new)
            if diff_data:
                logging.warning("Alterações significativas detectadas na distribuição dos dados!")
            else:
                logging.info("Distribuição dos dados está estável.")
        else:
            logging.info("Não foram encontradas estatísticas anteriores para comparação.")

        # Salvar estatísticas atuais
        stats_df = pd.DataFrame(stats_new)
        stats_df.to_csv(stats_path)

        # Salvar o scaler para uso posterior
        joblib.dump(scaler, scaler_output_path)
        logging.info(f"Scaler salvo em '{scaler_output_path}'.")

        # Treinando o KMeans
        optimal_k = 4  # Valor do Elbow
        logging.info(f"Treinando KMeans com K={optimal_k} clusters...")
        kmeans = KMeans(n_clusters=optimal_k, random_state=42, n_init='auto')
        labels = kmeans.fit_predict(X_scaled)

        # Adicionar cluster ao dataframe
        df['Cluster'] = labels

        # Salvar CSV com clusters
        df.to_csv(output_csv_path, index=False)
        logging.info(f"Dados com clusters salvos em '{output_csv_path}'.")

        # Salvar modelo treinado
        joblib.dump(kmeans, model_output_path)
        logging.info(f"Modelo KMeans salvo em '{model_output_path}'.")

        # Mostrar centróides no espaço original
        centroids_original = scaler.inverse_transform(kmeans.cluster_centers_)
        centroids_df = pd.DataFrame(centroids_original, columns=features)
        logging.info(f"Centróides (valores originais):\n{centroids_df.to_string(index=True)}")

        # Carregar centróides antigos para comparar estabilidade
        if os.path.exists(centroids_path):
            logging.info("Carregando centróides anteriores para comparação...")
            old_centroids = np.load(centroids_path)
            drift_centroids = compare_centroids(old_centroids, centroids_original)
            if drift_centroids:
                logging.warning("Mudanças significativas detectadas nos centróides dos clusters!")
            else:
                logging.info("Centróides dos clusters estão estáveis.")
        else:
            logging.info("Não foram encontrados centróides anteriores para comparação.")

        # Salvar centróides atuais
        np.save(centroids_path, centroids_original)

    except FileNotFoundError:
        logging.error(f"Arquivo de entrada '{input_csv_path}' não encontrado.")
    except Exception as e:
        logging.error(f"Ocorreu um erro: {e}")


if __name__ == "__main__":
    main()
