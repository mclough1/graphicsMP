Land Name / Land Members
	King of Jojo's Bizzare Scape - Torvesta, Dio, Hank Hill
	
Assignment Number / Project Title
	MP - Roller Coaster Tycoon
	
A brief, high level description of what the program is / does
	Takes in a file to specify a bezier surface floor, a bezier curve roller coaster and placement of objects around the world.
	Three heroes are present, two go along the roller coaster at different intervals while one walks along the floor.
	There are also three camera angles to toggle between, first-person, arcball and a free cam.
	
A usage section, explaining how to run the program, which keys perform which actions, etc.
	ESC/Q - quit
	WASD + arrow keys - move character
	# - change camera
	#
	#
	#
	
Instructions on compiling your code
	type make run
	
NEW: A description of the file format that your program reads as input
	<number of Bézier Surfaces for ground>
	<sixteen control points for each surface>
	...
	<number of control points for Bézier Curve track>
	<control point x> <control point y> <control point z>
	...
	<number of objects>
	<object type> <object x, y, z> <object size>
	...
	
	object type (must be all caps) can be:
		CUBE
		SPHERE

NEW: A rough distribution of responsibilities / contributions of each student
	Trevor:
		Bezier Surface
	Matt:
		Cameras
		General Hero Class
	Lake:
		Roller Coaster
	Collective:
		Walking on bezier surface
		Each person implemented their own hero
		
How long did this assignment take you?
	20hrs
	
How much did the lab help you for this assignment? 1-10 (1 - did not help at all, 10 - this was exactly the same as the lab)
	10
	
How fun was this assignment? 1-10 (1 - discontinue this assignment, 10 - I wish I had more time to make it even better)
	2


