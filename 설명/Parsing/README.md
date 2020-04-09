#
Scanning의 결과 Token 배열이 생성된다. 

이제 만들어진 토큰 배열을 이용해서 트리를 구현할 차례이다.

#
    class Type // has name
    class ItemType : public Type // has data.   ( name = data ) or (data (name is empty string))
    class UserType : public Type // has std::vector<ItemType>, and std::vector<UserType*>
#
Sequential Parsing.
    - stack is needed.

    if state == 0 && now char is LEFT( { )
        -> make UserType, stack.push(made UserType)
    else if state == 0 && now char is RIGHT( } )
        -> if stack`s size > 1 then stack.pop()
        -> if stack`s size is one  then  make Virtual Node. stack.pop(), stack.push(made virtual node)
    else if state == 0 && next char is ASSIGNMENT( = )
        -> pass now char, pass next char(=). state = 1
    else if state == 0
        -> val
    else if state == 1 && now char is LEFT( { )
        -> make UserType, state = 0
    else if state == 1
        -> var = val, state = 0
#
Sequential Parsing에서 VirutalNode가 생기면 text가 unvalid하다는 뜻. ex) { } }

그리고 파싱하고난후 stack의 size가 1보다 커도 text가 valid하지않다는 뜻이다.


또한 일반? 문자열이 와야할 var, val에  { } = 이 오면 역시 text가 unvalid(문법에 어긋나는?) 한 경우이다.

#
사실 ClauParer에서 Sequential Parsing을 하든 Parallel Parsing을 하든 위의 알고리즘?를 이용한다.

 즉, 단순히 동시에 여러개를 돌리는 것일뿐이다. 다만, 동작전이나 동작후에 에러처리나 토큰 나누기 등 여러가지 코드를 
 
추가할 필요가 있다. 
