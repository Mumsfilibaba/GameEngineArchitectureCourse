To create a package, simply make sure that the define at the top of "GameAssign2.cpp" called "CREATE_PACKAGE" is defined when compiling. In order to then
load and test the created package, comment the "CREATE_PACKAGE" define and recompile.

Make sure that the "Resources" directory is located wherever the compiled program is located, if not using Visual Studio.

Visualizing the resource manager.
If the user wants to change the state of a specific resource the user can do so. 
Green represents a resource in use
Yellow represents a resource only loaded.
Red represents a resource not loaded.
By left-clicking the resource name the user gets the option to change the current state of the resource. 
If the user try to load a resource that is of larger size than available the resource manager will try to unload not used resources. 
If this is not possible an error will appear in the console.  