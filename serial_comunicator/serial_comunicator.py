﻿#umi vycist port, baudrate, timeout a endline z dokumentu .txt a endline pridava na konec dat
#nepokracuje, pokud tan neni =, nepokracuje, pokud to, co a byt cislo neni cislo 
#muze byt pred a za = mezera (i vice)
#nepokracuje, pokud pred nebo za = neni zadny data_dictionary
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
    data1 = ""
    i = 0
    while i < lenght:
        if data[i] == "\\" and i < lenght and data[i+1] in escape_sequences:
            data1 += escape_sequences[data[i+1]]
            i += 2

        elif data[i] == "\\" and i < (lenght-2) and data[i+1] == "x" and data[i+2] in hex and data[i+3] in hex:
            hex_number = ""
            hex_number[0] = chr(int(data[i+2:i+4], 16))
            data1 += hex_number
            i += 4

        elif data[i] == "\\" and i < (lenght-2) and data[i+1] in octal and data[i+2] in octal and data[i+3] in octal:
            octal_number = ""
            octal_number[0] = chr(int(data[i+1:i+4], 8))
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
        return -1

def data_processing():
    data_dictionary = {}#nejak inteligentne pojmenovat
    try:
        with open(os.path.join(os.path.dirname(__file__), 'config.txt')) as f:
            for i in range(4):
                cmd1 = f.readline()   
                if not "=" in cmd1:
                    print "v souboru neni ="
                    return -1

                cmd = cmd1.split("=", 1)
                cmd[0] = cmd[0].strip() #smaze bile znaky na konci a na zacatku radku
                cmd[-1] = cmd[-1].strip()

                if cmd[0] == '':
                    print "Neni klic do slovniku"
                    return -1

                if cmd[-1] == '':
                    print "Neni, co ulozit do slovniku"
                    return -1

                if "#" in cmd[-1]:
                    i = 0
                    cmd2 = ""
                    while i < len(cmd[-1]):
                        if cmd[-1][i] == "#":
                            break
                        cmd2 += cmd[-1][i]
                        i += 1
                    cmd[-1] = cmd2
    
                data_dictionary[cmd[0]] = cmd[-1] 
    
    except IOError:
        print "nelze otevrit soubor"
        return -1
       
    print data_dictionary
    return data_dictionary

def communication(data):
    data_dictionary = data_processing() 
    
    if "port" in data_dictionary:
        cmd = data_dictionary["port"] 
      
    else:
        print "musis zadat port!"
        return -1 
    
    if "baudrate" in data_dictionary:
        v = data_dictionary["baudrate"]
        v = to_number(v, int)

    else:
        v = 115200

    if "endline" in data_dictionary:
        endline = data_dictionary["endline"]
        data += endline

    else:
        endline = ""

    if "timeout" in data_dictionary:
        wait = data_dictionary["timeout"]  
        wait = to_number(wait, float)

    else:
        wait = None

    data = ' '.join(data[1:]) 
    data1 = escape_sequention(data)
    return v
    ser = serial.Serial(cmd, v, timeout = wait)#pythonhosted.org/pyserial/pyserial_api.html
    x = ser.write(data1)
    ser.close()
    print 'closed'
    return x

if __name__ == '__main__':
    #data = sys.argv
    data = str(raw_input('Zadejte data '))
    end = communication(data)  
    print end
    os.system("pause")
    exit(end)