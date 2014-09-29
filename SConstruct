reader_target = "swmr-reader"
reader_sources = ['src/swmr-reader.cpp']
writer_target = "swmr-writer"
writer_sources = ['src/swmr-writer.cpp']


env = Environment()
env.Append(CCFLAGS = ['-g'])

# Add the dependency of log4cxx
env.Append(LIBS=['log4cxx', 'hdf5', 'rt'])
env.Append(CPPPATH=['hdf5swmr/include'])
env.Append(LIBPATH=['hdf5swmr/lib'])

#env.Append(LIBPATH=['/opt/local/lib'])
#env.Append(CPPPATH=['/opt/local/include'])

reader = env.Program(target = reader_target, source = reader_sources)
writer = env.Program(target = writer_target, source = writer_sources)

env.Install(dir = "/usr/local/bin", source = reader)
env.Install(dir = "/usr/local/bin", source = writer)
env.Alias('install', ['/usr/local/bin'])

