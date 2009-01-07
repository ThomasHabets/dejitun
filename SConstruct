# dejitun/SConstruct
env = Environment(CCFLAGS='-Wall -W -g -O2 -pipe',CPPPATH='.')

env.Program('dejitun',Split('''
dejitun.cc
inet.cc
tun_linux.cc
tun_freebsd.cc
tun_solaris.cc
util.cc
'''))
