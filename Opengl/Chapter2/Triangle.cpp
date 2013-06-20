// Triangle.cpp
// Our first OpenGL program that will just draw a triangle on the screen.

#include <GLTools.h>            // OpenGL toolkit
#include <GLShaderManager.h>    // Shader Manager Class
#include <GLFrustum.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>

#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif

GLBatch	pyramidBatch;
GLShaderManager	shaderManager;
GLFrustum viewFrustum;
GLMatrixStack projectionMatrix;
GLMatrixStack modelviewMatrix;
GLFrame cameraFrame;
GLFrame objectFrame;
GLGeometryTransform transformationPipeline;

float vCyan[] = {0.0f, 1.0f, 1.0f, 1.0f};
float vBlack[] = {0.0f, 0.0f, 0.0f, 1.0f};

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h)
{
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelviewMatrix.LoadIdentity();
}


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context. 
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC()
	{
	// Blue background
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f );
    
	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	cameraFrame.MoveForward(-15.0f);

	transformationPipeline.SetMatrixStacks(modelviewMatrix, projectionMatrix);

	// Load up a triangle
	GLfloat vPoints[100][3];    // Scratch array, more than we need
    int nVerts = 0;
    GLfloat r = 3.0f;
    vPoints[nVerts][0] = 0.0f;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = 0.0f;

    for(GLfloat angle = 0; angle < M3D_2PI; angle += M3D_2PI / 6.0f) {
        nVerts++;
        vPoints[nVerts][0] = float(cos(angle)) * r;
        vPoints[nVerts][1] = float(sin(angle)) * r;
        vPoints[nVerts][2] = -2.0f;
        }

    // Close the fan
    nVerts++;
    vPoints[nVerts][0] = r;
    vPoints[nVerts][1] = 0;
    vPoints[nVerts][2] = 0.0f;

	pyramidBatch.Begin(GL_TRIANGLE_FAN, 8);
	pyramidBatch.CopyVertexData3f(vPoints);
	pyramidBatch.End();
	}



///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void drawWireFrameBatch(GLBatch* pBatch)
 {
	 glEnable(GL_MULTISAMPLE);
    // Draw the batch solid green
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformationPipeline.GetModelViewProjectionMatrix(), vCyan);
    pBatch->Draw();
    
	glDisable(GL_MULTISAMPLE);
    // Draw black outline
    glPolygonOffset(-1.0f, -1.0f);      // Shift depth values
    glEnable(GL_POLYGON_OFFSET_LINE);

    // Draw lines antialiased

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    
    // Draw black wireframe version of geometry
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(3.0f);
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformationPipeline.GetModelViewProjectionMatrix(), vBlack);
    pBatch->Draw();
    
    // Put everything back the way we found it
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    }

void RenderScene(void)
	{
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	modelviewMatrix.PushMatrix();
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelviewMatrix.MultMatrix(mCamera);

	M3DMatrix44f mObject;
	objectFrame.GetMatrix(mObject);
	modelviewMatrix.MultMatrix(mObject);

	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformationPipeline.GetModelViewProjectionMatrix(), vCyan);

	drawWireFrameBatch(&pyramidBatch);

	// Perform the buffer swap to display back buffer

	modelviewMatrix.PopMatrix();
	glutSwapBuffers();
	}

void SpecialKeys(int key, int x, int y)
    {
	if(key == GLUT_KEY_UP)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    
	if(key == GLUT_KEY_DOWN)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
	
	if(key == GLUT_KEY_LEFT)
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    
	if(key == GLUT_KEY_RIGHT)
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    
	glutPostRedisplay();
    }


///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
	{
	gltSetWorkingDirectory(argv[0]);
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Triangle");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
	glutSpecialFunc(SpecialKeys);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
		}
	SetupRC();

	glutMainLoop();
	return 0;
	}