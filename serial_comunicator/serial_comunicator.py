#umi vycist port, baudrate, timeout a endline z dokumentu .txt a endline pridava na konec dat
#nepokracuje, pokud tan neni =, nepokracuje, pokud to, co a byt cislo neni cislo 
#muze byt pred a za = mezera (i vice)
#vypne se, pokud pred nebo za = neni zadny text
import serial, sys, os

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
    data1 = []
    i = 0
    while i <= (lenght - 1):
        if data[i] == "\\" and i < lenght and data[i+1] in escape_sequences:
            data1 += escape_sequences[data[i+1]]
            i += 2

        elif data[i] == "\\" and i < (lenght-2) and data[i+1] == "x" and data[i+2] in hex and data[i+3] in hex:
            hex_number = [""]
            hex_number[0] = chr(int(data[i+2] + data[i+3], 16))
            data1 += hex_number
            i += 4

        elif data[i] == "\\" and i < (lenght-2) and data[i+1] in octal and data[i+2] in octal and data[i+3] in octal:
            octal_number = [""]
            octal_number[0] = chr(int(data[i+1] + data[i+2] + data[i+3], 8))
            data1 += octal_number
            i += 4

        else:
            data1 += data[i]
            i += 1
    
    return data1

def to_number(number, numer_type):
    print number
    try:
        return numer_type(number)

    except ValueError:
        print "neni to cislo"
        os.system("pause")
        exit(-1)
        #return False #tady potom chybovou hlasku, ze to musi byt cislo  

def communication():
    #data = sys.argv
    data = str(raw_input('Zadejte data '))
    data = ' '.join(data[1:])
    text = {}#nejak inteligentne pojmenovat
    f = open(os.path.join(os.path.dirname(__file__), 'config.txt'))
    #jak zjistim kolik radku je popsanych v souboru .txt
    for i in range(4):
        cmd1 = f.readline()
        cmd1 = cmd1.strip()#smaze bile znaky na konci a na zacatku radku
            
        if not "=" in cmd1:
            print "v souboru neni ="
            os.system("pause")
            exit(-1)

        cmd = cmd1.split("=")
        cmd[0] = cmd[0].strip()
        cmd[-1] = cmd[-1].strip()
        if cmd[0] == '':
            print "Neni klic do slovniku"
            os.system("pause")
            exit(-1)
        if cmd[-1] == '':
            print "Neni, co ulozit do slovniku"
            os.system("pause")
            exit(-1)

        text[cmd[0]] = cmd[-1] 
       
    f.close
    print text
    
    if "port" in text:
        cmd = text["port"]  
    
    if "baudrate" in text:
        v = text["baudrate"]
        v = to_number(v, int)

    if "endline" in text:
        endline = text["endline"]
        data += endline

    if "timeout" in text:
        wait = text["timeout"]  
        wait = to_number(wait, float)

    print endline
    print cmd
    print v
    print wait
    data1 = escape_sequention(data)
    os.system("pause")
    return wait
    ser = serial.Serial(cmd, v, timeout = wait)
    x = ser.write(data1)
    ser.close()
    print 'closed'
    return x

if __name__ == '__main__':
    exit(communication())