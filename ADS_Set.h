#ifndef ADS_SET_H
 #define ADS_SET_H

 #include <functional>
 #include <algorithm>
 #include <iostream>
 #include <stdexcept>

 template <typename Key, size_t N = 7>
 class ADS_set {
 public:
     class Iterator;
     using value_type = Key;
     using key_type = Key;
     using reference = key_type &;
     using const_reference = const key_type &;
     using size_type = size_t;
     using difference_type = std::ptrdiff_t;
     using iterator = Iterator;
     using const_iterator = Iterator;
     using key_equal = std::equal_to<key_type>; // Hashing
     using hasher = std::hash<key_type>;        // Hashing

 private:
     enum class Mode {free, used, freeagain,end};
     struct element {
         key_type key;
         Mode mode {Mode::free};
         element *next{nullptr};
     };
     element *table{};
     size_type table_size {0}, curr_size {0};
     float max_lf {0.7};
     size_type h(const key_type &key) const { return hasher{}(key) % table_size; }
     size_type g(const key_type &, size_type pos) const { return (pos + 1) % table_size; }
     void reserve(size_type n);
     void rehash(size_type n);
     element *insert_(const key_type &key);
     element *find_(const key_type &key) const;
     void addNode(const size_type&, const key_type&) const;
 public:
     ADS_set() { rehash(N); }
     ADS_set(std::initializer_list<key_type> ilist): ADS_set{} { insert(ilist); }
     template<typename InputIt> ADS_set(InputIt first, InputIt last): ADS_set{} { insert(first,last); }
     ADS_set(const ADS_set &other): ADS_set{} {
         reserve(other.curr_size);
         for (const auto &k: other) {    
             insert_(k);
         }
     }

     ~ADS_set(){
         destroyer(table);
         delete[] table;
     }

     void destroyer(element*);

     ADS_set &operator=(const ADS_set &other){
         if (this == &other) return *this;
         ADS_set tmp{other};
         swap(tmp);
         return *this;
     }

     ADS_set &operator=(std::initializer_list<key_type> ilist) {
         ADS_set tmp{ilist};
         swap(tmp);
         return *this;
     }
     size_type size() const { return curr_size; }
     bool empty() const { return curr_size == 0; }

     size_type count(const key_type &key) const { return !!find_(key); }

     iterator jumpedFrom(element *from, element *to)const{
         iterator a{from}; iterator b{to};
         while(a != b){
             ++a;
         }
         return a;
     }

     iterator find(const key_type &key) const{
         if(auto p {find_(key)}){
             if(p->next == nullptr){
                 if(key_equal{}(p->key, key) && p->mode == Mode::used) { return iterator{p}; }
             }
             else{
                 element *temp = p;
                 while(p->next != nullptr){
                     if(key_equal{}(p->key,key) && p->mode == Mode::used){return jumpedFrom(temp, p);}
                     p = p->next;
                 }
                 if(key_equal{}(p->key, key) && p->mode == Mode::used ){ return jumpedFrom(temp, p); }
             }
         }
         return end();
     }

     void clear(){
         ADS_set tmp;
         swap(tmp);
     }

     void swap(ADS_set &other) {
         using std::swap;
         swap(table, other.table);
         swap(curr_size, other.curr_size);
         swap(table_size, other.table_size);
         swap(max_lf, other.max_lf);
     }

     void insert(std::initializer_list<key_type> ilist) { insert(std::begin(ilist), std::end(ilist)); }

     //!HIERR
     std::pair<iterator,bool> insert(const key_type &key){
         if(auto p{find_(key)}) return {find(key), false};
         reserve(curr_size+1);
         return {iterator{insert_(key)}, true};
     }

     template<typename InputIt> void insert(InputIt first, InputIt last);

     size_type erase(const key_type &key){
         if(auto temp {find_(key)}){

             if(temp->next == nullptr){
                 if(key_equal{}(temp->key, key)){temp->mode = Mode::freeagain; --curr_size; return 1;}
             }
             else{
                 while(temp->next != nullptr){
                     if(key_equal{}(temp->key,key)){
                         temp->mode = Mode::freeagain;
                         --curr_size;
                         return 1;
                     }
                     temp = temp->next;
                 }
                 if(key_equal{}(temp->key, key)){temp->mode = Mode::freeagain; --curr_size; return 1;}
             }
         }
         return 0;
     }

     const_iterator begin() const{  return const_iterator{table}; }
     const_iterator end() const{ return const_iterator{table+table_size}; }

     void dump(std::ostream &o = std::cerr) const;

     friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
         if (lhs.curr_size != rhs.curr_size) return false;
         for (const auto &k: rhs) if (!lhs.count(k)) return false;
         return true;
     }

     friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs){return !(lhs == rhs);}
 };

 template <typename Key, size_t N>
 typename ADS_set<Key,N>::element *ADS_set<Key,N>::insert_(const key_type &key) {
     size_type idx {h(key)};


     if(table[idx].mode != Mode::used){
         table[idx].key = key;
         table[idx].mode = Mode::used;
     }
     else{
         element *temp = new element();
         *temp = table[idx];
         table[idx].key = key;
         table[idx].mode = Mode::used;
         table[idx].next = temp;
     }

     ++curr_size;
     return table+idx;
 }

 template <typename Key, size_t N>
 typename ADS_set<Key,N>::element *ADS_set<Key,N>::find_(const key_type &key)const{
     size_type idx {h(key)};

     //? VLLT KANN ICH HIER SPÄTER WIEDER WAS OPTIMIEREN
     element temp = table[idx]; //free freeagain end used
     if(temp.mode == Mode::freeagain || temp.mode == Mode::used){
         if(temp.next == nullptr){
             if(key_equal{}(temp.key, key) && temp.mode == Mode::used) return table+idx;
         }
         else{
             while(temp.next != nullptr){
                 if(key_equal{}(temp.key,key) && temp.mode == Mode::used) return table+idx;
                 temp = *temp.next;
             }
             if(key_equal{}(temp.key, key)&& temp.mode == Mode::used) return table+idx;
         }
     }
     return nullptr;
 }

 template <typename Key, size_t N>
 template<typename InputIt> void ADS_set<Key,N>::insert(InputIt first, InputIt last) {
     for (auto it {first}; it != last; ++it) {
         if (!count(*it)) {
             reserve(curr_size+1);
             insert_(*it);
         }
     }
 }

 template <typename Key, size_t N>
 void ADS_set<Key,N>::reserve(size_type n) {
     if (n > table_size * max_lf) {
         size_type new_table_size {table_size};
         do {
             new_table_size = new_table_size * 2 + 1;
         } while (n > new_table_size * max_lf);
         rehash(new_table_size);
     }
 }

 template <typename Key, size_t N>
 void ADS_set<Key,N>::rehash(size_type n) {
     size_type new_table_size {std::max(N, std::max(n,size_type(curr_size / max_lf)))};
     element *new_table {new element[new_table_size+1]};
     new_table[new_table_size].mode = Mode::end; 
     size_type old_table_size {table_size};
     element *old_table {table};
     curr_size = 0;
     table = new_table;
     table_size = new_table_size;
     for (size_type idx{0}; idx < old_table_size; ++idx) {

         if(old_table[idx].mode == Mode::freeagain || old_table[idx].mode == Mode::used){

             if(old_table[idx].next == nullptr){
                 if(old_table[idx].mode == Mode::used){
                     insert_(old_table[idx].key);
                 }
             }
             else{
                 element temp = old_table[idx];
                 while(temp.next != nullptr){
                     if(temp.mode == Mode::used){
                         insert_(temp.key);
                     }
                     temp = *temp.next;
                 }
                 if(temp.mode == Mode::used) insert_(temp.key);
             }
         }

     } //!ENDE FOR

     for (size_type i = 0; i < old_table_size; i++){
         element temp = old_table[i];
         if(temp.next == nullptr){
             delete temp.next;
         }
         else{
             while(temp.next != nullptr){
                 element temp1 = temp;
                 temp = *temp.next;
                 delete temp1.next;
             }
         }
         delete temp.next;
     }

     delete[] old_table;
 }

 template <typename Key, size_t N>
 void ADS_set<Key,N>::dump(std::ostream &o) const {
     o << "curr_size = " << curr_size << " table_size = " << table_size << "\n";
     for (size_type idx{0}; idx < table_size+1; ++idx) {
         o << idx << ": ";
         element temp = table[idx];

         if(temp.next == nullptr){
             switch(temp.mode){
                 case Mode::free:
                     o << "--free"; break;
                 case Mode::used:
                     o << temp.key; break;
                 case Mode::end:
                     o << "--END"; break;
                 case Mode::freeagain:
                     o << "--freeagain"; break;
             }
         }
         else{
             while(temp.next != nullptr){
                 switch(temp.mode){
                     case Mode::free:
                         o << "--free"; break;
                     case Mode::used:
                         o << temp.key; break;
                     case Mode::end:
                         o << "--END"; break;
                     case Mode::freeagain:
                         o << "--freeagain"; break;
                 }
                 if(temp.next != nullptr){ o << " -> ";}
                 temp = *temp.next;
             }
             switch(temp.mode){
                 case Mode::free:
                     o << "--free"; break;
                 case Mode::used:
                     o << temp.key; break;
                 case Mode::end:
                     o << "--END"; break;
                 case Mode::freeagain:
                     o << "--freeagain"; break;
             }
         }
         o << "\n";
     }
 }

 template <typename Key, size_t N>
 void ADS_set<Key,N>::destroyer(element* toDestroy) {
     for (size_type i = 0; i < table_size; i++){
         element temp = toDestroy[i];
         if(temp.next == nullptr){
             delete temp.next;
         }
         else{
             while(temp.next != nullptr){
                 element temp1 = temp;
                 temp = *temp.next;
                 delete temp1.next;
             }
         }
         delete temp.next;
     }
 }



 template <typename Key, size_t N>
 class ADS_set<Key,N>::Iterator {
 public:
     using value_type = Key;
     using difference_type = std::ptrdiff_t;
     using reference = const value_type&;
     using pointer = const value_type*;
     using iterator_category = std::forward_iterator_tag;
 private:
     element *pos;
     element *end{nullptr};
     element *beforeJump{nullptr};
     bool posSprung{false};
     bool jumpBack{false};
     bool first{false};
     void skip(){

         while(pos->mode == Mode::free && pos->mode != Mode::end){
             ++pos;
         }

         if(first && pos->mode == Mode::freeagain){ //für den fall dass das erste element in der liste einen freeagain ist
             if(pos->next == nullptr){
                 ++pos;
                 skip();
             }
             else{

                 pos_end();
                 pos = pos->next;
                 if(pos == end){ jumpBack = true;}
                 if(pos->mode == Mode::freeagain) ++*this;
             }
             first = false;
         }
     }

     void pos_end(){
         beforeJump = pos;
         end = pos;
         posSprung = true;
         while(end->next != nullptr)
             end = end->next;
     }

     void return_to_normal(){
         pos = beforeJump;
         beforeJump = nullptr;
         posSprung = false;
         end = nullptr;
         jumpBack = false;
     }

     bool isAJUMP{false};

 public:
     explicit Iterator(element *pos = nullptr) : pos{pos}{
         first = true;
         if(pos) skip();


     }
     reference operator*() const{ return pos->key; }
     pointer operator->() const{ return &pos->key; }
     Iterator& operator++(){

         if(jumpBack){
             return_to_normal();
             ++pos;
             skip();
             if(pos->mode == Mode::freeagain) {++*this;}
             return *this;
         }

         if(pos->next == nullptr){
             ++pos;
             skip();
             if(pos->mode == Mode::freeagain) {++*this;}
             return *this;
         }
             //5 -> 10 -> 13
         else{
             if(!posSprung){
                 pos_end();
             }
             pos = pos->next;
             if(pos == end){
                 jumpBack = true;
             }
             if(pos->mode == Mode::freeagain) {++*this;}
             return *this;
         }
     }

     Iterator operator++(int){
         auto rc {*this};
         if(pos->mode != Mode::end){++*this;}
         return rc;
     }

     friend bool operator==(const Iterator &lhs, const Iterator &rhs){return lhs.pos == rhs.pos;}
     friend bool operator!=(const Iterator &lhs, const Iterator &rhs){return !(lhs == rhs);}
 };

 template <typename Key, size_t N> void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

 #endif // ADS_SET_H 
