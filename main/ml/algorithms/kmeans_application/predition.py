import os
import pandas as pd
import joblib
from sklearn.preprocessing import StandardScaler

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# Caminhos
model_path = os.path.join(BASE_DIR, 'models', 'kmeans_model.pkl')
input_csv_path = os.path.join(BASE_DIR, '../data/processed/sensor_data_cleaned.csv')
output_csv_path = os.path.join(BASE_DIR, '../data/processed/sensor_data_with_clusters.csv')

# 1. Carregar modelo
kmeans = joblib.load(model_path)

# 2. Carregar dados
df_new = pd.read_csv(input_csv_path)
features = ['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM']
X_new = df_new[features]

# 3. Aplicar mesmo scaler usado no treinamento
scaler = StandardScaler()
X_new_scaled = scaler.fit_transform(X_new)

# 4. Prever clusters
df_new['PredictedCluster'] = kmeans.predict(X_new_scaled)

# 5. Exibir e salvar
print(df_new[['Temperature(C)', 'Humidity(%)', 'MQ4_PPM', 'MQ7_CO_PPM', 'PredictedCluster']].head())
df_new.to_csv(output_csv_path, index=False)
print(f"Clusters salvos em: {output_csv_path}")
