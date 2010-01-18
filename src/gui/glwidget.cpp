// OpenGL implementation in Qt
// Parts of this are blantantly ripped off from BSNES (thanks Byuu!)
//
// by James L. Hammons
// (C) 2010 Underground Software
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/14/2010  Created this file
//

#include "glwidget.h"

#include "settings.h"

GLWidget::GLWidget(QWidget * parent/*= 0*/): QGLWidget(parent), texture(0),
	textureWidth(0), textureHeight(0), buffer(0), rasterWidth(64), rasterHeight(64)
//	textureWidth(0), textureHeight(0), buffer(0), rasterWidth(256), rasterHeight(256)
{
}

GLWidget::~GLWidget()
{
}

void GLWidget::initializeGL()
{
	format().setDoubleBuffer(true);
	resizeGL(rasterWidth, rasterHeight);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_DITHER);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void GLWidget::paintGL()
{
	unsigned outputWidth  = width();
	unsigned outputHeight = height();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, outputWidth, 0, outputHeight, -1.0, 1.0);
	glViewport(0, 0, outputWidth, outputHeight);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (smoothGLOutput ? GL_LINEAR : GL_NEAREST));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (smoothGLOutput ? GL_LINEAR : GL_NEAREST));
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST /*GL_LINEAR*/);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST /*GL_LINEAR*/);
//	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rasterWidth, rasterHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rasterWidth, rasterHeight, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, buffer);

	double w = (double)rasterWidth  / (double)textureWidth;
	double h = (double)rasterHeight / (double)textureHeight;
	unsigned u = outputWidth;
	unsigned v = outputHeight;

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(0, 0); glVertex3i(0, v, 0);
	glTexCoord2f(w, 0); glVertex3i(u, v, 0);
	glTexCoord2f(0, h); glVertex3i(0, 0, 0);
	glTexCoord2f(w, h); glVertex3i(u, 0, 0);
	glEnd();
}

void GLWidget::resizeGL(int width, int height)
{
	if (width > textureWidth || height > textureHeight)
	{
//		textureWidth  = max(width,  textureWidth);
//		textureHeight = max(height, textureHeight);
// Seems that power of 2 sizes are still mandatory...
		textureWidth  = 1024;//(width > textureWidth ? width : textureWidth);
		textureHeight = 512;//(height > textureHeight ? height : textureHeight);
//		textureWidth  = (width > textureWidth ? width : textureWidth);
//		textureHeight = (height > textureHeight ? height : textureHeight);
#if 0
printf("Resizing: new texture width/height = %i x %i\n", textureWidth, textureHeight);
printf("Resizing: new raster width/height = %i x %i\n", rasterWidth, rasterHeight);
#endif

		if (buffer)
		{
			delete[] buffer;
			glDeleteTextures(1, &texture);
		}

		buffer = new uint32_t[textureWidth * textureHeight];
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, textureWidth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);
	}
}

#if 0
class RubyGLWidget: public QGLWidget
{
  public:
    GLuint texture;
    unsigned textureWidth, textureHeight;

    uint32_t * buffer;
    unsigned rasterWidth, rasterHeight;

    bool synchronize;
    unsigned filter;

    void updateSynchronization() {
      #ifdef __APPLE__
      makeCurrent();
      CGLContextObj context = CGLGetCurrentContext();
      GLint value = synchronize;  //0 = draw immediately (no vsync), 1 = draw once per frame (vsync)
      CGLSetParameter(context, kCGLCPSwapInterval, &value);
      #endif
    }
} * widget;
#endif