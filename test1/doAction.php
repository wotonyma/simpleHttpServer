<?php
header('content-type:text/html;charset=utf-8');
require_once 'functions/mysql.func.php';
require_once 'config/config.php';
require_once 'swiftmailer-master/lib/swift_required.php';
require_once 'functions/common.func.php';
//接受信息
$act=$_REQUEST['act'];
$username=$_REQUEST['username'];
$password=isset($_POST['password'])?$_POST['password']:'';
$link=connect3();
$table='51zxw_user';
//根据用户不同操作完成不同功能
switch($act){
	case 'reg':
		$username=addslashes($username);
		$query="insert into {$table} (username, password) values('{$username}', '{$password}')";
		$res = mysqli_query ( $link, $query );
		if ($res) {
		alertMes('注册成功，跳转到首页','index.php');
		} else {
		alertMes('注册错误，重新注册','index.php');
		}
		
		break;
		
	case 'checkUser':
		$username=mysqli_real_escape_string($link, $username);
		$query="SELECT id FROM {$table} WHERE username='{$username}'";
		$row=fetchOne($link, $query);
		if($row){
			echo 1;
		}else{
			echo 0;
		}
		break;
	case 'login':
		$username=addslashes($username);
		$query="SELECT id FROM {$table} WHERE username='{$username}' AND password='{$password}'";
		$row=fetchOne($link, $query);
		if($row){
		    alertMes('登陆成功，跳转到首页','student/layout-index.php');

		}else{
			alertMes('用户名或密码错误，重新登陆','index.php');
		}
		break;
	default:
		die('非法操作');
		break;
}
