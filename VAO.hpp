#ifndef VAO_H
#define VAO_H
	
class VAO  
{
private:

	public:

		unsigned int ID;

		VAO();
		void bind();
		void set(unsigned int index, unsigned int size, unsigned int type, unsigned int stride, const void *pointer);
		static void unbind();

		~VAO();

};
#endif