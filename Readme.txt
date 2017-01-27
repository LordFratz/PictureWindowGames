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



Shadow Mapping is mostly complete, but with some bugs or lackings:
-Map is only storing 8 bit unsigned ints instead of intended floats (causing depth check issues in the shadow shader, as only 1 or -1 seem to be found in the texture)
-Does not work with mage model, or the plane

-Stream output fully functional
-Does 3 draw calls:
Animation code only runs once!
1st: Skinning Vertex Shader, Null GS, PS with void return
	**Creating a GeometryShader with stream output using vertex shader bytecode enables 
	  the Stream Output stage without the Geometry shader stage (bypassing two primitive 
          constructions)
2nd: Vertex pass through shader, Instancing GS, PS that writes to shadowmap floating point depths from light's perspective & view
3rd: As 2nd, except PS does lighting and texturing instead, sampling depth from the shadowmap

Ideally, the plane would have been moved into the new lighting system, the shadowmap depth writing fixed, 
and the 2nd and 3rd pass above would be combined by either taking instancing out of geometry shader and 
using DrawIndexInstanced, or by using multiple stream output targets with the geometry shader to allow light point of view depth writing and SO in 1 call.