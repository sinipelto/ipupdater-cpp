TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -L$$PWD/lib -lcurl

copydir.commands = $(COPY_DIR) $$shell_path($$PWD/skeleton) $$shell_path($$OUT_PWD)
first.depends = $(first) copydir
export(first.depends)
export(copydir.commands)
QMAKE_EXTRA_TARGETS += first copydir

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
    utils.hh \
    ip.hh

SOURCES += \
    internal/CurlHandle.cpp \
    internal/OptionList.cpp \
    internal/OptionSetter.cpp \
    internal/SList.cpp \
    cURLpp.cpp \
    Easy.cpp \
    Exception.cpp \
    Form.cpp \
    Info.cpp \
    ip.cpp \
    main.cpp \
    Multi.cpp \
    OptionBase.cpp \
    Options.cpp \
    utils.cpp
