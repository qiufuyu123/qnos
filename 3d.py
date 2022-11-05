
import math
import time
from tkinter import  *
root = Tk()
root.geometry('320x240')
mycanvas = Canvas(root,bg='green',height=200,width=280)
mycanvas.pack()
cur_ax=0
def rotX():
    global cur_ax
    cur_ax=0
    print(cur_ax)
def rotY():
    global cur_ax
    print(cur_ax)
    cur_ax=1
def rotZ():
    global cur_ax
    print(cur_ax)
    cur_ax=2
BX=Button(root,text='X',command=rotX)
BY=Button(root,text='Y',command=rotY)
BZ=Button(root,text='Z',command=rotZ)
BX.pack()
BY.pack()
BZ.pack()
vertexs=[
    [1,1,2],[3,1,2],[1,3,2],[3,3,2],
    [1,1,4],[3,1,4],[1,3,4],[3,3,4]
    ]
oripoint=[2,2,3]
lines=[[0,1,3,2,0],[4,5,7,6,4],[0,4],[1,5],[2,6],[3,7]]
eye_x,eye_y,eye_z=1.5,1.5,1
def transto2d(vt):
    global eye_x,eye_y,eye_z
    outy=(eye_z*(vt[1]-eye_y))/(eye_z+(vt[2]))+eye_y
    outx=(eye_z*(vt[0]-eye_x))/(eye_z+(vt[2]))+eye_x
    return [outx,outy]

def transcube(vt):
    res=[]
    for i in range(8):
        res.append(transto2d(vt[i]))
    return res
def rotate(x,y,a,px,py):
    return [math.cos(a)*(x-px)-math.sin(a)*(y-py)+px,math.cos(a)*(y-py)+math.sin(a)*(x-px)+py]
# 0:x 1:y 2:z

def rotatevertexs(vt,asix,a,p):
    res=[]
    for i in range(8):
        if asix==1:
            o=rotate(vt[i][0],vt[i][2],a,p[0],p[2])
            res.append([o[0],vt[i][1],o[1]])
        elif asix==0:
            o=rotate(vt[i][1],vt[i][2],a,p[1],p[2])
            res.append([vt[i][0],o[0],o[1]])
        elif asix==2:
            o=rotate(vt[i][0],vt[i][1],a,p[0],p[1])
            res.append([o[0],o[1],vt[i][2]])
    return res
def rendercube(vt,ls):
    for  i in ls:
        for j in range(len(i)-1):
            mycanvas.create_line(vt[i[j]][0]*90,vt[i[j]][1]*90,vt[i[j+1]][0]*90,vt[i[j+1]][1]*90,fill='red')
pa=1
def update():
    global pa,cur_ax
    mycanvas.delete('all')
    if pa>=360:
        pa=1
    pa=pa+1
    rendercube(transcube(rotatevertexs(vertexs,cur_ax,pa,oripoint)),lines)
    root.after(200,update)
print(transcube(vertexs))
root.after(200,update)
root.mainloop()
time.sleep(1)

