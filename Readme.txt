Folder Name: Graphics1Hardware
Solution: Graphics1Hardware.sln
FBX SDK Version: fbx 2016 1.1 vs2015 win
Visual Studio Version: 2015
Platform: X64
Controls
    Movement WASD, X -> up, C -> down, right click and drag to look around
	Animation: (When in manual mode) O -> cycle forward, P -> cycle backward, I -> switch to animated mode
			   (When in animated mode) O/P -> switch to manual mode
			   
To render bear instead of box:
	Compile with LOADED_BEAR as 1 for bear, 0 for box (line 35 in main.cpp)

To switch between fill and wireframe:
	Use the 'F' key
	
To switch between Animations
	1, 2, 3, 4
	NOTE: Once it's blending between animations it will not 
	accept another call to change animations until it is finished