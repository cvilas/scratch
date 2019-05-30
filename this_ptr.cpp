// https://stackoverflow.com/questions/993352/when-should-i-make-explicit-use-of-the-this-pointer

template<class T>
struct A 
{
   int i;
};
 
template<> 
struct A<float> 
{ 
    float x; 
};

template<class T>
struct B : A<T> 
{
    int foo() 
    {
        return this->i; //!< good.
        //return i; //!< Bad. will not compile
        
        /// Claim: If you omit this->, the compiler does not know how to treat i, 
        /// since it may or may not exist in all instantiations of A. In order to 
        /// tell it that i is indeed a member of A<T>, for any T, the this-> prefix is required.
    }
 
};
 
int main() 
{
    B<int> b;
    
    // B<float> bb; // will not compile, bacause A<float> has no member i.
    
    b.foo();
}
