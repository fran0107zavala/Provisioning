import socket
import sys
import subprocess
import re

# CONFIGURACIÓN
DEVICE_IP = "192.168.1.1"
DEVICE_PORT = 15000
SOCKET_TIMEOUT = 10
EXPECTED_AP_SSID = "MQTT_Device"  # ← CAMBIADO

# Credenciales a enviar
SSID = "GalaxyS25UltradeDavid"
PASSWORD = "holadavid"


def get_current_ssid():
    """Obtiene el SSID actual al que está conectado (solo Windows)"""
    try:
        result = subprocess.run(
            ['netsh', 'wlan', 'show', 'interfaces'],
            capture_output=True,
            text=True,
            encoding='cp850'  # Codificación para Windows en español
        )

        for line in result.stdout.split('\n'):
            if 'SSID' in line and ':' in line:
                # Extrae el SSID
                ssid = line.split(':', 1)[1].strip()
                if ssid and not 'BSSID' in line:
                    return ssid
    except Exception as e:
        print(f"[!] No se pudo detectar el SSID: {e}")
    return None


def get_gateway_ip():
    """Obtiene la IP del gateway actual"""
    try:
        result = subprocess.run(
            ['ipconfig'],
            capture_output=True,
            text=True,
            encoding='cp850'
        )

        lines = result.stdout.split('\n')
        for i, line in enumerate(lines):
            if 'Puerta de enlace' in line or 'Gateway' in line:
                # Extraer IP de la siguiente línea o de esta línea
                ip_match = re.search(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})', line)
                if ip_match:
                    return ip_match.group(1)
    except Exception as e:
        print(f"[!] No se pudo detectar gateway: {e}")
    return None


def validate_credentials(ssid, password):
    """Valida que las credenciales sean correctas"""
    if not ssid or len(ssid) > 32:
        return False, "SSID inválido o muy largo (máx 32 caracteres)"

    if password:
        if len(password) < 8:
            return False, "Password muy corto (mínimo 8 caracteres para WPA)"
        if len(password) > 63:
            return False, "Password muy largo (máximo 63 caracteres)"

    return True, "OK"


def send_credentials(device_ip, ssid, password):
    """Envía credenciales WiFi al dispositivo"""

    # Validar antes de enviar
    valid, msg = validate_credentials(ssid, password)
    if not valid:
        print(f"[!] Error de validación: {msg}")
        return False

    # Preparar datos
    data = f"{ssid},{password}"

    try:
        print(f"[*] Conectando a {device_ip}:{DEVICE_PORT}...")

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(SOCKET_TIMEOUT)
        s.connect((device_ip, DEVICE_PORT))

        print(f"[*] ✓ Conectado!")
        print(f"[*] Enviando credenciales...")
        print(f"[*]   SSID: {ssid}")
        print(f"[*]   Password: {'*' * len(password)}")

        s.sendall(data.encode('utf-8'))

        print("[*] Esperando respuesta del dispositivo...")
        ack = s.recv(1024).decode('utf-8').strip()

        print(f"[*] Respuesta: '{ack}'")

        s.close()

        if ack == "OK":
            print("\n[✓] ¡Provisionamiento exitoso!")
            print("[*] El dispositivo ahora intentará conectarse a la red Wi-Fi")
            return True
        else:
            print(f"\n[!] Respuesta inesperada del dispositivo: '{ack}'")
            return False

    except socket.timeout:
        print("\n[!] ERROR: Timeout - El dispositivo no respondió")
        print("[!] Posibles causas:")
        print("    1. El servidor de provisioning no está activo")
        print("    2. La IP del dispositivo es incorrecta")
        print("    3. Hay un firewall bloqueando la conexión")
        return False

    except ConnectionRefusedError:
        print("\n[!] ERROR: Conexión rechazada")
        print("[!] El puerto 10001 no está escuchando en el dispositivo")
        return False

    except socket.error as e:
        print(f"\n[!] ERROR de socket: {e}")
        return False

    except Exception as e:
        print(f"\n[!] ERROR inesperado: {e}")
        return False


def main():
    print("=" * 70)
    print("  Script de Provisionamiento WiFi")
    print("=" * 70)
    print()

    # Detectar SSID actual
    current_ssid = get_current_ssid()
    if current_ssid:
        print(f"[*] SSID actual detectado: '{current_ssid}'")

        if current_ssid != EXPECTED_AP_SSID:
            print(f"[!] ERROR: Debes estar conectado a '{EXPECTED_AP_SSID}'")
            print(f"[!] Actualmente estás conectado a '{current_ssid}'")
            print()
            print(f"[*] Por favor conéctate al AP del dispositivo:")
            print(f"    SSID: {EXPECTED_AP_SSID}")
            print(f"    Password: 012345678")
            sys.exit(1)
        else:
            print(f"[✓] Conectado al AP correcto!")
    else:
        print("[!] No se pudo detectar el SSID actual")
        print(f"[!] Asegúrate de estar conectado a: {EXPECTED_AP_SSID}")
        input("Presiona ENTER para continuar de todos modos... ")

    # Detectar gateway
    gateway_ip = get_gateway_ip()
    if gateway_ip:
        print(f"[*] Gateway detectado: {gateway_ip}")
        device_ip = gateway_ip
    else:
        print(f"[!] No se pudo detectar gateway, usando IP por defecto: {DEVICE_IP}")
        device_ip = DEVICE_IP

    print()
    print("[*] Iniciando provisionamiento...")
    print()

    success = send_credentials(device_ip, SSID, PASSWORD)

    if success:
        print("\n[✓] Proceso completado exitosamente")
        sys.exit(0)
    else:
        print("\n[!] El provisionamiento falló")
        print("\n[*] Pasos de diagnóstico:")
        print(f"    1. Verifica que estés conectado a '{EXPECTED_AP_SSID}'")
        print(f"    2. Prueba hacer ping: ping {device_ip}")
        print(f"    3. Reinicia el dispositivo y espera 10 segundos")
        print(f"    4. Verifica que el dispositivo muestre '[prov] Provisioning server listening'")
        sys.exit(1)


if __name__ == "__main__":
    main()