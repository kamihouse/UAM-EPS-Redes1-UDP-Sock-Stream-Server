################################################################################
# UAM - EPS - Ingeniería de Tecnologías y Servicios de Telecomunicación
# Server: client_udp.c
# File..: makefile
# Link..: https://github.com/kamihouse/UDP-Sock-Stream-Server
#
# Trabajo de practica Arquitectura de Redes I - 10/11/2012
# Profesores..:	Pedro María Santiago del Río (pedro.santiago@uam.es)
#		Javier Ramos (javier.ramos@uam.es)
#
# Alumnos.....: Thiago Pereira Rosa	(thiago.pereirarosa@estudiante.uam.es)
#		Isaac Gonzalez Gonzalez	(isaac.gonzalez@estudiante.uam.es)
################################################################################

# banderas de compilacion (PUEDE SER UTIL CAMBIARLAS)
CC = gcc
#ifdef ENTREGA
CFLAGS = -Wall -g -I .
LDLIBS = -lm
#else
CFLAGS = -Wall -g -I .
#-DENTREGA
LDLIBS = -lm
#endif

# fuentes a considerar (si no se cambia, todos los '.c' del directorio actual)
SOURCES = $(shell ls -1 *.c* | xargs)

# busca los ejecutables (todos los .c con metodo tipo 'int main')
EXEC_SOURCES = $(shell grep -l "^int main" $(SOURCES) | xargs)

# fich. de dependencia (.d) y nombres de ejecutables (los anteriores, sin '.c')
EXECS = $(shell echo $(EXEC_SOURCES) | sed -e 's:\.c[p]*::g')
DEPS = $(shell echo $(SOURCES) | sed -e 's:\.c[p]*:\.d:g')

all:	udpLookup

# las dependencias (CAMBIA PARA CADA PRACTICA)
client_udp:	udpLookup.o

# receta para hacer un .d (dependencias automaticas de tipo .h para tus .o)
%.d : %.c
	@set -e; $(CC) -MM $(CFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@

#incluye las dependencias generadas
-include $(DEPS)

# receta para hacer un .o
%.o :	%.c
	@echo -n compilando objeto \'$<\'...
	@$(CC) $(CFLAGS) $< -c
	@echo [OK]

# receta para hacer un ejecutable (suponiendo resto de dependencias OK)
% :	%.o
	@echo -n compilando ejecutable \'$@\'...
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	@echo [OK]

# limpieza
clean:
	@rm -f $(wildcard *.o *.d core* *.P) $(EXECS)

# ayuda (nunca viene mal)
help:
	@echo "Use: make <target> ..."
	@echo "Valid targets:"
	@$(MAKE) --print-data-base --question | sed -e "s:makefile::g" |\
	awk '/^[^.%][-A-Za-z0-9_]*:/	\
		{ print "   " substr($$1, 1, length($$1)-1) }'