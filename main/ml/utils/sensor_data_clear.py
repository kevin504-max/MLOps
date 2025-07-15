import pandas as pd
import logging

# Configuração básica de logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

# Input and output file paths
input_file = '../data/raw/merged.csv'
output_file = '../data/processed/sensor_data_cleaned.csv'

def remove_outliers_iqr(df, features):
    """
    Remove outliers usando o método IQR (Interquartile Range).
    Retorna o dataframe filtrado.
    """
    Q1 = df[features].quantile(0.25)
    Q3 = df[features].quantile(0.75)
    IQR = Q3 - Q1
    mask = ~((df[features] < (Q1 - 1.5 * IQR)) | (df[features] > (Q3 + 1.5 * IQR))).any(axis=1)
    return df[mask]

def clean_and_sort_sensor_data(input_path: str, output_path: str):
    """
    Loads a CSV file with sensor readings, removes rows where any numeric sensor values
    are 0.0 or NaN, removes outliers via IQR, sorts by timestamp, and writes cleaned data.

    Logs key steps and statistics for monitoring.
    """
    logging.info(f"Starting to load data from {input_path}")
    df = pd.read_csv(input_path, parse_dates=['Timestamp'])
    logging.info(f"Data loaded successfully: {len(df)} rows")

    numeric_columns = df.columns.drop('Timestamp')

    initial_rows = len(df)

    # Remove linhas com qualquer valor 0.0 ou NaN em colunas numéricas
    mask_any_zero_or_nan = df[numeric_columns].isna().any(axis=1) | df[numeric_columns].eq(0.0).any(axis=1)
    df_filtered = df[~mask_any_zero_or_nan]
    logging.info(f"Removed {initial_rows - len(df_filtered)} rows with any zero or null sensor values")

    # Remove outliers pelo método IQR
    before_outlier_removal = len(df_filtered)
    df_no_outliers = remove_outliers_iqr(df_filtered, numeric_columns)
    logging.info(f"Removed {before_outlier_removal - len(df_no_outliers)} outliers using IQR method")

    # Ordena por Timestamp
    df_sorted = df_no_outliers.sort_values(by='Timestamp')
    logging.info("Data sorted by Timestamp")

    # Salva arquivo limpo
    df_sorted.to_csv(output_path, index=False)
    logging.info(f"Cleaned sensor data saved to: {output_path}")


if __name__ == "__main__":
    clean_and_sort_sensor_data(input_file, output_file)
