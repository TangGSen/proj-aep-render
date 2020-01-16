#include "gl_aux.h"
#include "AEPFilter.h"



CAEPFilter::CAEPFilter(char* szVertex, char* szFragment)
{
	_szVertex = szVertex;
	_szFragment = szFragment;

	_framebuffer = -1;
	_textureOut = -1;
	_textureIn = -1;
	_VAO = -1;
	_pShader = NULL;
	_width = 0;
	_height = 0;
}

CAEPFilter::~CAEPFilter()
{

}

int CAEPFilter::Open(int width, int height)
{
	_width = width;
	_height = height;

	_framebuffer = createFramebuffer(width, height);
	if (_framebuffer == 0)
	{
		return -1;
	}
	
	_pShader = new CShader(_szVertex, _szFragment);
	if (_pShader == NULL)
	{
		cout << "new shader fail..." << endl;
		return -2;
	}
	
	_VAO = makeVAO(_pShader->getProgramID());

	return 0;
}

int CAEPFilter::Close()
{
	return 0;
}

int CAEPFilter::Render(unsigned int textureIn)
{
	return Render(-1, textureIn);
}

int CAEPFilter::Render(float fProgress, unsigned int textureIn)
{
	// bind to framebuffer and draw scene as we normally would to color texture 
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glViewport(0, 0, _width, _height);
	//glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// bind Texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureIn);

	_pShader->use();
	if (fProgress>=0)
		_pShader->setFloat("fProgress", fProgress);
	glBindVertexArray(_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return 0;
}


unsigned int CAEPFilter::makeVAO(unsigned int programID)
{
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// positions          // colors           // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0 - 1.0f, // top right
		 1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0 - 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0 - 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0 - 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(glGetAttribLocation(programID, "aPos"), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(glGetAttribLocation(programID, "aPos"));
	// color attribute
	glVertexAttribPointer(glGetAttribLocation(programID, "aColor"), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(glGetAttribLocation(programID, "aColor"));
	// texture coord attribute
	glVertexAttribPointer(glGetAttribLocation(programID, "aTexCoord"), 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(glGetAttribLocation(programID, "aTexCoord"));

	return VAO;
}

unsigned int CAEPFilter::createFramebuffer(int width, int height)
{
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// create a color attachment texture
	glGenTextures(1, &_textureOut);
	glBindTexture(GL_TEXTURE_2D, _textureOut);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureOut, 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
		cout << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
		return 0;
	}
		
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return framebuffer;
}

