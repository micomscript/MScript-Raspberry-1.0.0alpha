#Makefile

TARGET = mscript




mscript: expression.o format.o function.o logic.o main.o parser.o resource.o sentence.o token.o tokenizer.o varmap.o function_m.o lock.o observer.o BME280.o -lpthread
        g++ -g parser.cpp expression.cpp format.cpp sentence.cpp logic.cpp main.cpp resource.cpp function.cpp token.cpp tokenizer.cpp varmap.cpp function_m.cpp lock.cpp observer.cpp BME280.cpp  -lwiringPi -lpthread -o mscript







parser.o: parser.cpp
        g++ $< -c -o $@

expression.o: expression.cpp
        g++ $< -c -o $@

format.o: format.cpp
        g++ $< -c -o $@

function.o: function.cpp
        g++ $< -c -o $@

logic.o: logic.cpp
        g++ $< -c -o $@

main.o: main.cpp
        g++ $< -c -o $@

resource.o: resource.cpp
        g++ $< -c -o $@

sentence.o: sentence.cpp
        g++ $< -c -o $@

token.o: token.cpp
        g++ $< -c -o $@

tokenizer.o: tokenizer.cpp
        g++ $< -c -o $@

varmap.o: varmap.cpp
        g++ $< -c -o $@

function_m.o: function_m.cpp
        g++ $< -c -o $@
#~/wiringpi-latest.deb -lwiringPi

lock.o: lock.cpp
        g++ $< -c -o $@

observer.o: observer.cpp
        g++ $< -c -o $@

BME280.o: BME280.cpp
        g++ $< -c -o $@
#command.o: command.cpp
#       g++ $< -c -o $@

clean:
        rm main.o
