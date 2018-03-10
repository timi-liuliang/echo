NVIDIA APEX 1.3 SDK distribution

Visual Studio based APEX projects:
----------------------------------
To build the Visual Studio based APEX projects in this distribution, the 
PhysX SDK path in the nxpath.vsprops files must be modified.  The 
UpdatePhysXPaths.vbs script is provided for this purpose.  It will
modify the nxpath.vsprops files under the compiler folders.

The 3.x PhysX path should contain the folder location (absolute or relative)
that contains the "Include" directory.

The 2.8.4 PhysX path should contain the folder location (absolute or relative)
that contains the "SDKs" directory.


Makefile based APEX projects:
-----------------------------
To build the makefile based APEX projects, the PhysX SDK path and other 
undefined paths near the top of the Makefile (in either the compiler or 
samples\compiler folders) must be modified.

APEX SDK and APEX Samples Makefile:
 * NDKROOT - path to the NDK root folder
 * NXPATH - path to the PhysX SDK root folder

APEX Samples Makefile: 
 * JAVA_HOME - path to the JDK root folder
 * ANT_TOOL - path to the ant tool script file
 
Edit the local.properties file and change the sdk.dir line to point to your 
Android SDK folder (forward slashes or double back-slashes)
 * /samples/AndroidActivities/Simple[Destruction, Clothing]/local.properties