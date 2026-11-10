# PR pictures: colour themes and Now Playing faces

These pictures go with the colour themes and Now Playing faces pull request (`feat/more-themes`, commit `b780588`). This branch holds only the pictures and is not meant to be merged.

Every picture comes from the firmware's own drawing code. The app runs on the test fake HAL, and each frame is read back pixel by pixel and scaled 2x with nearest neighbour. The track names and the cover art are invented for the pictures.
