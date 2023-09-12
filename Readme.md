
Procedural Generation Project Using the BSP(Binary Space Partition) Base<br/>
use assets file Content.zip in Content folder

--- asset list in gitignore ---
Content/Assets<br/>
Content/FirstPerson<br/>
Content/StarterContent<br/>
Content/Geometry<br/>

FirstPerson is dummy asset<br/>
StarterContent and Geometry are Unreal Basic Assets<br/>
custom asset is only material in Content/Assets.<br/>


Process
1) Binary Space Partition (generate random size block) <br/>
2) Connect the selected blocks using Delaunay Trianulation <br/>
3) Make Minimum spanning Tree (MST) using Prim algorithm <br/>
4) (Optional) add extra edges to form cycle <br/>
5) Select nearest spots each blocks <br/>
6) Use A* algorithm to make hallways <br/>

*Can use additional DFS algorithm to count number of hops if you hope to distinguish each rooms' sequence <br/>


![PCG_Proto](https://user-images.githubusercontent.com/80544647/230785173-d45cec34-596c-45a6-91b4-76c60cc7ab4c.png)

![PCG-MidProto](https://user-images.githubusercontent.com/80544647/230785194-9543ed24-0dcf-4fd9-9a48-e72bf3d8b7a9.PNG)

![image](https://user-images.githubusercontent.com/80544647/230923827-15ef6b0d-754a-4b95-95b0-1217181a693c.png)

![image](https://user-images.githubusercontent.com/80544647/232234361-a32d3022-f7f9-4121-afca-823170fe11a2.png)

![Reincorporate](https://user-images.githubusercontent.com/80544647/232480934-ad918f2d-dcf6-4b54-ad4f-086181a72397.PNG)

![image](https://user-images.githubusercontent.com/80544647/233895154-b64fba69-c75e-49e4-8f43-dffbd9980a87.png)

![image](https://user-images.githubusercontent.com/80544647/234505590-cdcb0d96-f741-45e6-be19-d42ee1cc778f.png)

![image](https://user-images.githubusercontent.com/80544647/234838742-18c7ea82-2fb1-41d0-ad99-12fe5575eab9.png)
