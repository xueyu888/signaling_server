# signaling_server (Webrtc-based peerconnection server)

#master branch
extracting the code of the signaling server of the WebRTC
  
#test branch
作用：TCP穿透测试信令服务器（其实就是稍微改一点点webrtc的东西）
目标：固定一台内网本机监听TCP端口，其它对端的内网电脑与本机进行TCP穿透测试，观察是否能够成功穿透。
要求：
1 本机的端口是可变的，注册到信令服务器时，服务器应当能够标识本机的不同性，便于后续机子能够指定连接到本机。暂定为在o属性中定义固定的username，为了避免username重复，再判断一下端口和私网IP。 



