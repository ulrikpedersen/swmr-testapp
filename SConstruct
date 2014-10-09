import glob

swmr_target = "swmr"
swmr_sources = glob.glob('src/*.cpp')

env = Environment()
env.Append(CCFLAGS = ['-g'])

# Add the dependency of log4cxx
env.Append(LIBS=['log4cxx', 'hdf5', 'rt', 'z', 'boost_program_options'])
env.Append(CPPPATH=['hdf5swmr/include'])
env.Append(LIBPATH=['hdf5swmr/lib'])

#env.Append(LIBPATH=['/opt/local/lib'])
#env.Append(CPPPATH=['/opt/local/include'])

swmr = env.Program(target = swmr_target, source = swmr_sources)

env.Install(dir = "/usr/local/bin", source = swmr)
env.Alias('install', ['/usr/local/bin'])

