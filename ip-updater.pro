TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -L$$PWD/lib -lcurl

libcopy.path = $$OUT_PWD/debug/
libcopy.files = $$PWD/lib/libcurl-x64.dll

INSTALLS += \
    libcopy

HEADERS += \
    curl/curl.h \
    curl/curlver.h \
    curl/easy.h \
    curl/mprintf.h \
    curl/multi.h \
    curl/stdcheaders.h \
    curl/system.h \
    curl/typecheck-gcc.h \
    curl/urlapi.h \
    curlpp/internal/CurlHandle.hpp \
    curlpp/internal/OptionContainer.hpp \
    curlpp/internal/OptionContainerType.hpp \
    curlpp/internal/OptionList.hpp \
    curlpp/internal/OptionSetter.hpp \
    curlpp/internal/SList.hpp \
    curlpp/cURLpp.hpp \
    curlpp/Easy.hpp \
    curlpp/Exception.hpp \
    curlpp/Form.hpp \
    curlpp/Info.hpp \
    curlpp/Infos.hpp \
    curlpp/Multi.hpp \
    curlpp/Option.hpp \
    curlpp/OptionBase.hpp \
    curlpp/Options.hpp \
    curlpp/Types.hpp \
    utilspp/clone_ptr.hpp \
    utilspp/EmptyType.hpp \
    utilspp/NonCopyable.hpp \
    utilspp/NullType.hpp \
    utilspp/SmartPtr.hpp \
    utilspp/ThreadingFactoryMutex.hpp \
    utilspp/ThreadingSingle.hpp \
    utilspp/TypeList.hpp \
    utilspp/TypeTrait.hpp \
    ip.h

SOURCES += \
    curlpp/internal/CurlHandle.inl \
    curlpp/internal/OptionContainer.inl \
    curlpp/internal/OptionSetter.inl \
    curlpp/Easy.inl \
    curlpp/Info.inl \
    curlpp/Option.inl \
    internal/CurlHandle.cpp \
    internal/OptionList.cpp \
    internal/OptionSetter.cpp \
    internal/SList.cpp \
#    utilspp/ThreadingFactoryMutex.inl \
#    utilspp/ThreadingSingle.inl \
    cURLpp.cpp \
    Easy.cpp \
    Exception.cpp \
    Form.cpp \
    Info.cpp \
    ip.cpp \
    main.cpp \
    Multi.cpp \
    OptionBase.cpp \
    Options.cpp
