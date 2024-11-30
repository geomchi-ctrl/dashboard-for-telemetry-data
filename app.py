from flask import Flask, request, render_template, jsonify
import openpyxl
from openpyxl import Workbook
import os

app = Flask(__name__)

# Armazenar dados de telemetria
telemetry_data = {
    "current": 0.0,
    "speed": 0,
    "motor_state1": "Desligado",
    "motor_state2": "Desligado",
    "wifi-state": "Desligado"
}

# Nome do arquivo Excel
excel_file = "telemetry_data.xlsx"

# Função para inicializar o arquivo Excel
def initialize_excel():
    if not os.path.exists(excel_file):
        workbook = Workbook()   
        sheet = workbook.active
        sheet.title = "Telemetria"
        # Cabeçalhos
        sheet.append(["Corrente (A)", "Speed", "Motor 1", "Motor 2", "Conexão WiFi"])
        workbook.save(excel_file)
        print(f"Arquivo Excel criado: {excel_file}")

# Função para salvar dados no Excel
def save_to_excel(data):
    workbook = openpyxl.load_workbook(excel_file)
    sheet = workbook.active
    # Adiciona os dados na próxima linha disponível
    sheet.append([
        data['current'],
        data['speed'],
        data['motor_state1'],
        data['motor_state2'],
        data['wifi-state']
    ])
    workbook.save(excel_file)
    print("Dados salvos no Excel.")

@app.route('/update', methods=['POST'])
def update():
    #Recebe dados de telemetria da ESP32 e os atualiza no servidor.
    global telemetry_data
    try:
        data = request.get_json()
        print(f"Dados recebidos: {data}")
        
        telemetry_data['current'] = data.get('current', telemetry_data['current'])
        telemetry_data['speed'] = data.get('speed', telemetry_data['speed'])
        telemetry_data['motor-state1'] = data.get('motor-state1', telemetry_data['motor-state1'])
        telemetry_data['motor-state2'] = data.get('motor-state2', telemetry_data['motor-state2'])
        telemetry_data['wifi-state'] = data.get('motor_state', telemetry_data['wifi-state'])

        # Salvar os dados no Excel
        save_to_excel(telemetry_data)

        return 'Data received', 200
    except Exception as e:
        print(f"Erro ao processar os dados: {e}")
        return 'Error processing data', 400

@app.route('/get_telemetry', methods=['GET'])
def get_telemetry():
    #Retorna os dados de telemetria no formato JSON.
    return jsonify(telemetry_data)

@app.route('/')
def index():
    #Renderiza o dashboard HTML.
    return render_template('index.html', data=telemetry_data)

if __name__ == '__main__':
    initialize_excel()
    app.run(host='0.0.0.0', port=8080, debug=True)
