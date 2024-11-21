import serial
import time

def init_serial_port():
    return serial.Serial(
        port='/dev/serial0',
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1)

def get_temperature():
    try:
        ser = init_serial_port()
        ser.reset_output_buffer()
        ser.reset_input_buffer()
        print("Port ouvert")

        # Envoi de la commande avec retour chariot
        ser.write(b'GET_T\r')
        print("Commande envoyée")
        time.sleep(0.2)

        print(f"Données disponibles : {ser.in_waiting} bytes")
        if ser.in_waiting:
            response = ser.readline().decode('utf-8').strip()
            print(f"Données brutes : {response}")
            return response
        return "No response from device"

    except Exception as e:
        print(f"Erreur : {e}")
        return str(e)
    finally:
        ser.close()


def get_pressure():
    try:
        ser = init_serial_port()
        ser.reset_output_buffer()
        ser.reset_input_buffer()

        ser.write(b'GET_P\r')
        time.sleep(0.2)

        if ser.in_waiting:
            response = ser.readline().decode('utf-8').strip()
            return response
        return "No response from device"

    except Exception as e:
        return str(e)
    finally:
        ser.close()


## command dois être sous la forme b'COMMAND_NAME\r'
def send_command(command):
    try:
        ser = init_serial_port()
        ser.reset_output_buffer()
        ser.reset_input_buffer()

        ser.write(command)
        time.sleep(0.2)

        if ser.in_waiting:
            response = ser.readline().decode('utf-8').strip()
            return response
        return "No response from device"

    except Exception as e:
        return str(e)
    finally:
        ser.close()