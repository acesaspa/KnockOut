//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2019 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

//#ifdef RENDER_SNIPPET

#include <vector>

#include "physx\PxPhysicsAPI.h"

#include "SnippetRender.h"
#include "SnippetCamera.h"

#include <iostream>
#include <ctime>
#include <sstream>

using namespace physx;
using namespace std;

extern void initPhysics();
extern void stepPhysics();	
extern void cleanupPhysics();
extern void keyPress(unsigned char key, const PxTransform& camera);

extern PxScene* gScene;

std::string time_t_to_string(time_t t)
{
	std::stringstream sstr;
	sstr << t;
	return sstr.str();
}


int frame_count = 0;
char* frame_time;
char menu[80] = "FPS: 0";

void init_timer() {
	std::time_t result = std::time(NULL);
	std::string time2 = time_t_to_string(result);
	frame_time = new char[20];
	strcpy_s(frame_time,20, time2.c_str());
}

namespace
{
Snippets::Camera*	sCamera;

void motionCallback(int x, int y)
{
	sCamera->handleMotion(x, y);
}

void keyboardCallback(unsigned char key, int x, int y)
{
	if(key==27)
		exit(0);

	if(!sCamera->handleKey(key, x, y))
		keyPress(key, sCamera->getTransform());
}

void mouseCallback(int button, int state, int x, int y)
{
	sCamera->handleMouse(button, state, x, y);
}

void idleCallback()
{
	glutPostRedisplay();
}

void text()
{
	if (frame_count == 0) {
		init_timer();
	}

	frame_count += 1;

	//std::cout << frame_count << "\n";

	std::time_t result = std::time(NULL);

	std::string time = time_t_to_string(result);
	const char* cstr = time.c_str();

	//std::cout << cstr << "\n";
	//std::cout << frame_reset << "\n";

	if (strcmp(cstr, frame_time) != 0) {
		//frame_reset = cstr;
		strcpy_s(frame_time,20, cstr);
		//std::cout << strcmp(cstr, frame_time) << "::\n";
		std::cout << frame_count << "\n";
		sprintf_s(menu, "FPS: %d", frame_count);
		frame_count = 0;
		std::cout << menu << "\n";
	}

	//char menu[80];
	//strcpy_s(menu, "Have courage and be kind");
	int len;
	len = strlen(menu);

	glColor3f(1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(0, 600, 0, 600);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();

	glRasterPos2i(5, 5);


	for (int i = 0; i < len; ++i)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, menu[i]);
	}

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void renderCallback()
{
	stepPhysics();
	Snippets::startRender(sCamera->getEye(), sCamera->getDir());
	text();
	PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
	if(nbActors)
	{
		std::vector<PxRigidActor*> actors(nbActors);
		gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
		Snippets::renderActors(&actors[0], static_cast<PxU32>(actors.size()), true);
	}
	Snippets::finishRender();
}

void exitCallback(void)
{
	delete sCamera;
	cleanupPhysics();
}
}

void renderLoop()
{
	sCamera = new Snippets::Camera(PxVec3(10.0f, 10.0f, 10.0f), PxVec3(-0.6f,-0.2f,-0.7f));

	Snippets::setupDefaultWindow("KnockOut");
	Snippets::setupDefaultRenderState();

	glutIdleFunc(idleCallback);
	glutDisplayFunc(renderCallback);
	glutKeyboardFunc(keyboardCallback);
	glutMouseFunc(mouseCallback);
	glutMotionFunc(motionCallback);
	motionCallback(0,0);

	atexit(exitCallback);

	initPhysics();
	glutMainLoop();
}

//#endif
