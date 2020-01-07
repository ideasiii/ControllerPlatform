[TTS Server]
	IP: 175.98.119.121
	Port: 2312
	
[TTS CMP]
	Command ID: 
		tts_request : 0x00000058
		tts_response: 0x80000058
		
	Body: JSON Data Format
		tts_request Packet Body Example:
		{
			"user_id":"",
			"voice_id":0,
			"emotion":0,
			"text":"多型態角色語音智慧平台"
			"fm":	"1",
			"b":	"0.0",
			"r":	"1.25"
			"id":   "aaa";
			"total": "1";
			"sequence_num": "1";
		}
		
		tts_response Packet Body Example:
		{
			"status":0
			"wave":"http://54.199.198.94/tts/Wate.wav"
			"label":"http://54.199.198.94/label/aaa.tar.gz"
			"data": "x^x-pau+n=anH@x_x/A:0/B:x@x-x&x-x/C:3/D:0/E:x@x+x/F:2/G:0_0/H:x=x@1=1/J:9+4-1
					x^pau-n+anH=anH@1_3/A:0/B:3@1-2&1-9/C:3/D:0/E:2@1+4/F:2/G:0_0/H:9=4@1=1/J:9+4-1"
		}
		#註解#
		status:狀態碼
			0：成功
			1：User ID 錯誤
			2：Voice ID 錯誤
			3：WAVE 產生失敗
			4：系統錯誤
		voice_ id:
			-1: zip	
			-2: new temp lab (tomcat...)
			>=0: different model
		req_type:
			2: delete tmp lab (tomcat...)
			1: Update WordData
			
//=====================RAW to WAV ================================//
sox -t raw -b 16 -e signed-integer -r 16000 -c 1 *.raw duck.wav


