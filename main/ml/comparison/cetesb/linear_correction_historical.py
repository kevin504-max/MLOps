import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.linear_model import LinearRegression
from sklearn.metrics import r2_score
import os

def load_data():
    """Carrega os dados dos sensores e CETESB já corrigidos por média histórica"""
    try:
        # Supondo que os dados estão em data/processed/comparacao_corrigida.csv
        data_path = os.path.join('data', 'processed', 'comparacao_corrigida.csv')
        df = pd.read_csv(data_path)
        
        # Verifica se temos as colunas necessárias
        if 'CO_ppm_corrigido_sensores' not in df.columns or 'CO_ppm_corrigido_cetesb' not in df.columns:
            raise ValueError("Dados não contêm colunas 'CO_ppm_corrigido_sensores' e 'CO_ppm_corrigido_cetesb'")
            
        return df
    except Exception as e:
        print(f"Erro ao carregar dados: {e}")
        return None

def apply_linear_calibration(df):
    """Aplica calibração linear aos dados dos sensores"""
    X = df['CO_ppm_corrigido_sensores'].values.reshape(-1, 1)  # Dados dos sensores
    y = df['CO_ppm_corrigido_cetesb'].values                   # Dados de referência
    
    # Ajusta modelo de regressão linear
    model = LinearRegression()
    model.fit(X, y)
    
    # Aplica a calibração
    df['Sensores_Calibrados'] = model.predict(X)
    
    # Calcula métricas
    r2 = r2_score(y, df['Sensores_Calibrados'])
    coef = model.coef_[0]
    intercept = model.intercept_
    
    print(f"Parâmetros da calibração:")
    print(f"Coeficiente (a): {coef:.4f}")
    print(f"Intercepto (b): {intercept:.4f}")
    print(f"R²: {r2:.4f}")
    
    return df, model

def plot_comparison(df):
    """Gera gráfico comparativo antes e depois da calibração"""
    plt.figure(figsize=(12, 6))
    
    # Plot dados originais
    plt.plot(df.index, df['CO_ppm_corrigido_cetesb'], 'b-', label='CETESB (Referência)')
    plt.plot(df.index, df['CO_ppm_corrigido_sensores'], 'orange', linestyle='--', alpha=0.5, label='Sensores (Corrigido por média)')
    
    # Plot dados calibrados
    plt.plot(df.index, df['Sensores_Calibrados'], 'r-', label='Sensores (Calibrados)')
    
    plt.title('Comparação CO - Sensores vs CETESB (Antes e Depois da Calibração)')
    plt.xlabel('Índice de Tempo')
    plt.ylabel('CO (ppm)')
    plt.legend()
    plt.grid(True)
    
    # Salva o gráfico
    output_path = os.path.join('data', 'processed', 'comparacao_co_calibrado.png')
    plt.savefig(output_path)
    print(f"Gráfico salvo em: {output_path}")
    plt.close()

def save_calibrated_data(df):
    """Salva os dados calibrados"""
    output_path = os.path.join('data', 'processed', 'sensor_readings_calibrado.csv')
    df.to_csv(output_path, index=False)
    print(f"Dados calibrados salvos em: {output_path}")

def main():
    # Carrega dados
    df = load_data()
    if df is None:
        return
    
    # Aplica calibração linear
    df, model = apply_linear_calibration(df)
    
    # Gera e salva gráfico
    plot_comparison(df)
    
    # Salva dados calibrados
    save_calibrated_data(df)
    
    # Opcional: Salvar o modelo de calibração se for útil para aplicações futuras
    # from joblib import dump
    # dump(model, os.path.join('data', 'processed', 'calibration_model.joblib'))

if __name__ == "__main__":
    main()