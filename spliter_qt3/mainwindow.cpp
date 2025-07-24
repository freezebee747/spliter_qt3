#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent), textEdit(new CodeEditor(this)), syntaxChecker(parser)
{
	DirSingleton::GetInstance().SetDir("C:\\testfolder");

	setCentralWidget(textEdit);
	resize(800, 600);
	setWindowTitle("Document Editor");

	highlighter = new ASTHighlighter(textEdit->document());

	createActions();
	textEdit->setTabStopDistance(4 * textEdit->fontMetrics().horizontalAdvance(' '));

	QTimer* parser_timer = new QTimer(this);
	connect(parser_timer, &QTimer::timeout, this, &MainWindow::Analyze); // 슬롯 연결
	parser_timer->start(1000); // 1초마다 timeout 시그널 발생
}

MainWindow::~MainWindow(){
}

void MainWindow::createActions(){
	exitAct = new QAction("Exit", this);
	connect(exitAct, &QAction::triggered, this, &MainWindow::exitApp);
}

void MainWindow::exitApp() {
	close();
}

void MainWindow::parsingText() {
	QString text = textEdit->toPlainText();
	std::string str = text.toStdString();
	parser.parsing(str);
}

void MainWindow::Analyze() {
	//구문 분석 실행
	parsingText();
	ASTList = parser.Getnodes();
	std::vector<Comment> comments = parser.GetComment();
	// QPlainTextEdit에서 전체 텍스트 가져오기
	QString fullText = textEdit->toPlainText();

	// 줄 단위로 분할
	QStringList lines = fullText.split('\n');

	// QStringList → std::vector<QString> 변환
	std::vector<QString> sourceLines;
	sourceLines.reserve(lines.size());
	for (const auto& line : lines) {
		sourceLines.push_back(line);
	}

	// 하이라이터에 AST와 함께 전달
	highlighter->clearHighlighter();
	highlighter->setAST(ASTList, sourceLines);
	highlighter->setComment(comments, sourceLines);

	//의미 분석 실행
	syntaxChecker.SyntaxCheck();

	highlighter->setErrors(syntaxChecker.GetError(), sourceLines);
	
}