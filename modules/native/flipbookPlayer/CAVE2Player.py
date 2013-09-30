# Implements a simple player for the CAVE2 system using the flipbookPlayer module.
from omega import *
from flipbookPlayer import *

def run():
	# Do not playback on master node.
	if(not isMaster()):
		setImageLoaderThreads(12)

		movieDir = "/iridium_SSD/kreda/movies/anp_3d/" + getHostname() 
		#movieDir = "/iridium_SSD/kreda/movies/organic_3d/" + getHostname() 

		player = FlipbookPlayer.createAndInitialize();

		player.loadMultidir(movieDir + "/%04d/frame_%06d.jpg", 7676, 1, 2046, 0);
		#player.loadMultidir(movieDir + "/%04d/frame_%04d.jpg", 4000, 3, 2046, 0);
		player.setFramesToBuffer(600)
		player.setFrameTime(0.02)
		player.setNumGpuBuffers(2)
		player.setPlaybackBarVisible(True)
		player.play()
		player.loop(True)

#	# Setup soundtrack
#	se = getSoundEnvironment()
#	if se != None:
#		music = se.loadSoundFromFile('music', '/Users/evldemo/sounds/music/filmic.wav')
#		simusic = SoundInstance(music)
#		simusic.setPosition(Vector3(0, 2, -1))
#		simusic.setLoop(True)
#		simusic.setReverb(0.1)
#		simusic.setMix(0.1)
#		simusic.setWidth(18)
#		simusic.setVolume(0.1)
#		simusic.playStereo()

run()
