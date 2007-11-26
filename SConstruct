env = Environment(CCFLAGS='-Wall -w -g -O2',CPPPATH='.')

env.Program('dejitun',Split('''
dejitun.cc
inet.cc
tun_linux.cc
tun_freebsd.cc
util.cc
'''))
