g++ -lcurl -Ofast cURLpp.cpp Easy.cpp Exception.cpp Form.cpp Info.cpp ip.cpp main.cpp Multi.cpp OptionBase.cpp Options.cpp internal/CurlHandle.cpp internal/OptionList.cpp internal/OptionSetter.cpp internal/SList.cpp  -o updater -I .
mkdir bin
cp -R skeleton/* ./bin/
mv updater ./bin/
