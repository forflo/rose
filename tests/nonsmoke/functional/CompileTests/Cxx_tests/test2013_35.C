// namespace std 
// {
     template <class T> class vector {};
// }

template <class T>
class BaseFab
   {
     public:
          void setVal (T x);
          void maskLT (BaseFab<int>& mask ) const;
   };

#if 0
template <class T>
void
BaseFab<T>::maskLT (BaseFab<int>& mask) const
   {
     mask.setVal(0);
   }
#else
void
foo (BaseFab<int>& mask)
   {
     mask.setVal(0);
   }
#endif

BaseFab< vector<int> > hash;

