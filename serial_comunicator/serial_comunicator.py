#umi vycist port, baudrate, timeout a endline z dokumentu .txt a endline pridava na konec dat
#nepokracuje, pokud tan neni =
#nepokracuje, pokud to, co ma byt cislo neni cislo 
#muze byt pred a za = mezera (i vice)
#nepokracuje, pokud pred nebo za = neni zadny configuration
import serial, sys, os, socket

def escape_sequention(data):
    escape_sequences = {"a" : "\a",
                        "b" : "\b",
                        "f" : "\f",
                        "n" : "\n",
                        "r" : "\r",
                        "t" : "\t",
                        "v" : "\v",
                        "\\" :  "\\",
                        "'" : "\'",
                        "\"" : "\"",
                        "?" : "\?"}
   
    hex = "0123456789abcdefABCDEF"
    octal = hex[:8]
    lenght = len(data)
    data1 = ""
    i = 0
    while i < lenght:
        if data[i] == "\\" and i < lenght and data[i+1] in escape_sequences:
            data1 += escape_sequences[data[i+1]]
            i += 2

        elif data[i] == "\\" and i < (lenght-2) and data[i+1] == "x" and data[i+2] in hex and data[i+3] in hex:
            hex_number = ""
            hex_number = chr(int(data[i+2:i+4], 16))
            data1 += hex_number
            i += 4

        elif data[i] == "\\" and i < (lenght-2) and data[i+1] in octal and data[i+2] in octal and data[i+3] in octal:
            octal_number = ""
            octal_number = chr(int(data[i+1:i+4], 8))
            data1 += octal_number
            i += 4

        else:
            data1 += data[i]
            i += 1
    
    return data1

def to_number(number, numer_type):
    #print number
    try:
        return numer_type(number)

    except ValueError:
        print "neni to cislo"
        return -1

def configuration_processing():
    configuration = {}#nejak inteligentne pojmenovat
    try:
        with open(os.path.join(os.path.dirname(__file__), 'config.txt')) as f:
            for line in f:                 
                if "#" in line:
                    line = line.split("#")
                    line = line[0]

                if line == "":
                    continue

                if not "=" in line:
                    print "v souboru neni ="
                    return -1

                cmd2 = line.split("=", 1)
                cmd2[0] = cmd2[0].strip() #smaze bile znaky na konci a na zacatku radku
                cmd2[0] = cmd2[0].lower()#zmensi vsechny znaky z klice
                cmd2[-1] = cmd2[-1].strip()

                if cmd2[0] == '':
                    print "Neni klic do slovniku"
                    return -1

                if cmd2[-1] == '':
                    print "Neni, co ulozit do slovniku"
                    return -1
    
                configuration[cmd2[0]] = cmd2[-1] 
    
    except IOError:
        print "nelze otevrit soubor"
        return -1

    return configuration

def communication(data, configuration): 
    data = ' '.join(data[1:])     

    if "port" in configuration:
        port = configuration["port"] 
      
    else:
        print "musis zadat port!"
        return -1 
    
    v = 115200
    if "baudrate" in configuration:
        v = configuration["baudrate"]
        v = to_number(v, int)     

    if "endline" in configuration:
        endline = configuration["endline"]
        data += endline

    wait = None
    if "timeout" in configuration:
        wait = configuration["timeout"]  
        wait = to_number(wait, float)

    data1 = escape_sequention(data)

    try:
        if "address" in configuration:
            address = configuration["address"]
            port = to_number(port, int)
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((address, port))
            x = s.send(data1)
            s.close()
        

        #return v
        else:
            ser = serial.Serial(port, v, timeout = wait)#pythonhosted.org/pyserial/pyserial_api.html
            x = ser.write(data1)
            ser.close()

    except serial.serialutil.SerialException as e:
        print e.message  
        return -1
        
    except socket.error as e:
        print '{} ({})'.format(e.strerror, e.errno)

        return -1
    

    print 'closed', x
    return x

if __name__ == '__main__':
    data = sys.argv
    #data = raw_input('Zadejte data: ')
    configuration = configuration_processing() 
    end = communication(data, configuration)  
    #print end
    #os.system("pause")
    exit(end)