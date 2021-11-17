#ifndef TEXTURE2D_H
#define TEXTURE2D_H
	
class Texture2D  
{
	private:

	public:
		int width, height, nrChannels;

		unsigned int ID;

		Texture2D(const char* imagePath);
		void bind();
		
		~Texture2D();

};
#endif