# socket

220.149.244.211~214 까지 돌렸을 때

각 ip에서 돌릴 때 바꿔야 할 변수 : 
  router.c 는 공통으로 돌린다. (바꿀 것 없음)
  fileopen.h 는 #define my_num 1 부분에서 각각 0~3(220.149.244.211 - 214) 으로 설정한다.
  input1.txt ~ input4.txt 는 1~4(220.149.244.211  ~ 214)에 해당하는 파일로 fileopen.h 에 input.txt로 들어간다. (input1.txt->input.txt 로 바꿔서 넣어야 함.)
 
ROU_NUM 변수는 모든 라우터의 갯수(여기서는 4)
my_num 변수는 현재 아이피에 대한 나의 번호 0~3(220.149.244.211 ~ 214)

각 table은 CT[][]에 들어가 있음

