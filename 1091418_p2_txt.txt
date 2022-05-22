input():
一開始先讓使用者輸入N個entry
在這裡也會避免掉小於等於零和非2的次方的數字

init():
接下來就依照輸入的數字將依些資料結構初始化
像是N個預測器和計算這N個預測器的mispredicrion

read():
再來就是讀檔
將input裡面的PC、instruction、operand等東西讀進已經設好的資料結構裡面
待會會用到

exec():
最後就開始執行 
執行的一開始先弄一個用來寫檔的outFile
然後弄兩個變數 分別是pre和out
代表這預測器的結果(0 or 1)和實際操作的結果(會跳去第幾行 or 0)

for:
接下來就一行一行跑
每一行都會去跑一個_pre(i)和ALU(i)的function來讓pre和out存預測器的結果和實際的結果

/*------------------------------------------------------------*/
_pre(i)裡面會先把i %= N用來判斷看哪個entry
然後再來看這個predictor裡的3-bit history來決定看哪個counter
再看這個counter的index來判斷是taken or not taken

ALU(i)裡面會把operand做切割
然後再判斷這個instruction是什麼
再做相應的計算
同時也會改變register的值
如果是branch的話則會判斷會不會jump
會的話就return要跳的那一行
不會的話就return 0
/*------------------------------------------------------------*/

if:
如果結果不一樣的話就代表預測錯誤
就把misprediction++

output():
最後就把結果cout和寫進output裡面

update():
然後再更新predictor
如果out裡面非零的話
就把對應的counter++
並且把i值改成out - 1
也就是要跳去的那一行
減一是因為for完之後都會++i
所以要減一來平衡
如果out為零的話
則是把對應的counter--
最後更新3-bit history

之後就一直重複for迴圈直到跳出
