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
		}
		
		tts_response Packet Body Example:
		{
			"status":0
			"wave":"http://54.199.198.94/tts/Wate.wav"
		}
		#註解#
		status:狀態碼
			0：成功
			1：User ID 錯誤
			2：Voice ID 錯誤
			3：WAVE 產生失敗
			4：系統錯誤
				
			
//=====================RAW to WAV ================================//
sox -t raw -b 16 -e signed-integer -r 16000 -c 1 *.raw duck.wav


