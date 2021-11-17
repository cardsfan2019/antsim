#ifndef EBO_H
#define EBO_H
	
class EBO  
{
	private:

	public:

		unsigned int ID;

		EBO();
		void bind();
		void init(const void* data, unsigned int size, unsigned int usage);
		
		~EBO();

};
#endif