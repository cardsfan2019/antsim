#ifndef VBO_H
#define VBO_H
	
class VBO  
{
	private:

	public:

		unsigned int ID;

		VBO();
		void bind();
		void init(const void* data, unsigned int size, unsigned int usage);

		~VBO();

};
#endif