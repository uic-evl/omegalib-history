# flipbookPlayer
a multicore image-based player for cluster systems

TODO - description

## Python API
The `flipbookPlayer` module exposes one class `FlipbookPlayer` that represents the main player interface

### `FlipbookPlayer`
| **Method** | Description
|---|---|
static `FlipbookPlayer createAndInitialize()` | creates an instance of the flipbook player.
`load(string filePath, int totalFrames, int startFrame)` | Starts loading frames from a single directory.
`loadMultiDir(string filePath, int totalFrames, ...` `..., int startFrame, int startPerDir, int startDirIndex)` | Starts loading frames from multiple directories.
`play()` | starts playback. Actual playback will start after enough frames have been buffered.
`pause()` | Pauses playback. Another call to play will restart playback from the current frame.
`stop()` | Stops playback. Another call to play will restart playback from the beginning of the animation.
`loop(bool loop)` | When set to true, playback will loop.
`int getFramesToBuffer()`, `setFramesToBuffer(int frames)` | Sets the number of frames that should be kept ready for playback. If not enough frames are ready, playback will pause and wait until enogh frames are ready.
`bool isPlaybackBarVisible()` `setPlaybackBarVisible(bool visible)` | If set to true, will display a bar showing the current playback point, buffered frames and frames queued for buffering.
`int getNumGpuBuffers()` `setNumGpuBuffer(int num)` | The number of frames to be buffered on the GPU. Increasing the number of buffers increases the data transfer parallelization but also increases GPU memory usage. Usually 1 or 2 GPU buffers are enough.

### Using multithreaded image loading
If you use the flipbook player as-is, you may have difficulty playing back an image-based animation at 60fps. This is because image decoding can take a significant part of the 16 milliseconds that each frame has to display, leading to repeated buffering and jittery playback. 
A solution to this is to enable multithreaded image loading: in omegalib you can do this using the `setImageLoaderThreads(int threads)` global function. Ideally you should use as many threads as your machine physical cores. If the threads are not required (for instance because the image buffer is already full), they will idle without consuming CPU resources.

