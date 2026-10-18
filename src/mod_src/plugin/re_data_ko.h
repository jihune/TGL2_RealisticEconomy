/* Korean texts of the game that the plugin puts right as the game reads its files (re_data.c, REPLACES).
 * Written by analysis/scripts/ko_wording.py from analysis/notes/e19_korean_wording.md and modsLanguages/ko as
 * shipped: change the script or the note, not this file. The number after each piece is the proposal's in the
 * note's order. */

static const piece KO_BASETRANSLATE[] = {
    {"<openSheetXP>", "체험 시트", "경험 시트"}, /* 3 */
    {"<songBan>", "{{SONG}}는 영구적으로 금지됩니다.", "{{SONG}} 곡이 영구 금지되었습니다."}, /* 272 */
    {"<hourlyDecay>", "시간별 부패", "시간당 감소"}, /* 12 */
    {"<timeMonthsTranslate>", "{{MONTHS}}월", "{{MONTHS}}개월"}, /* 4 */
    {"<showHomeActions>", "홈 액션을 표시합니다.", "집 활동 표시."}, /* 179 */
    {"<showCurrentJobs>", "현재 작업 표시.", "현재 직업 표시."}, /* 6 */
    {"<showMarketProperties>", "시장 속성을 표시합니다.", "시장에 나온 부동산 표시."}, /* 7 */
    {"<noObjectAvailable>", "아니요 {{OBJECTNAME}}", "{{OBJECTNAME}} 없음"}, /* 5 */
    {"<difficultyBadStatusDecay>", "나쁜 상태 붕괴", "나쁜 상태 감소"}, /* 8 */
    {"<difficultyNeedsDecayRate>", "부패 필요", "욕구 감소"}, /* 9 */
    {"<difficultyNeedsDecayRateTip>", "시간이 지남에 따라 얼마나 빨리 부패해야 하는지에 영향을 줍니다. 부패하지 않으려면 0으로 설정합니다.", "시간이 지나면서 욕구가 얼마나 빨리 줄어드는지 정합니다. 줄어들지 않게 하려면 0으로 설정합니다."}, /* 10 */
    {"<difficultyPointsModifier>", "포인트 수정자", "포인트 배율"}, /* 186 */
    {"<lifetimeSymbol>", "생활", "평생"}, /* 180 */
    {"<statusAbbreviation>", "통계", "상태"}, /* 189 */
    {"<monthlyHoursApproxTitle>", "월간 약. 시간", "월 시간(약)"}, /* 128 */
    {"<needsDecayTitle>", "부패 필요", "욕구 감소"}, /* 11 */
    {"<preferencesTitle>", "환경 설정", "선호도"}, /* 14 */
    {"<prefImpactTitle>", "기본 설정 영향", "선호도 영향"}, /* 15 */
    {"<multipliersTitle>", "승수", "배율"}, /* 183 */
    {"<modifiersTitle>", "수정자", "보정"}, /* 184 */
    {"<cooldownTitle>", "쿨다운 월", "대기 개월 수"}, /* 190 */
    {"<skillModifierTitle>", "스킬 모디파이어", "스킬 보정"}, /* 185 */
    {"<estimateTitle>", "견적", "예상"}, /* 187 */
    {"<enablesTitle>", "활성화", "가능해지는 것"}, /* 188 */
    {"<obsoleteTitle>", "폐기되었습니다: {{OLDWORDS}}", "구식: {{OLDWORDS}}"}, /* 193 */
    {"<historyTitle>", "역사", "기록"}, /* 191 */
    {"<legalTitle>", "법률", "합법"}, /* 192 */
    {"<cashAccount>", "현금 계정", "현금 계좌"}, /* 197 */
    {"<alreadyMarkedForRent>", "이미 대여 표시됨", "이미 임대로 내놓음"}, /* 198 */
    {"<alreadyMarkedForSale>", "이미 판매 표시됨", "이미 매물로 내놓음"}, /* 199 */
    {"<inflationChartTitle>", "역사적 인플레이션 및 성장률", "과거 인플레이션과 성장률"}, /* 23 */
    {"<overallInflationIndex>", "전체 색인", "전체 지수"}, /* 22 */
    {"<redenominationShort>", "현지 통화는 {{LARGENUMBER}}:1로 표시됩니다!", "현지 통화가 {{LARGENUMBER}}:1로 화폐 개혁되었습니다!"}, /* 194 */
    {"<redenominationLong>", "현지 화폐가 리디노미네이션되면서 곳곳에서 혼란 {{LARGENUMBER}}:1! 정부 관계자들은 시민들에게 게임을 저장하고 재충전할 것을 권장합니다. 시장은 통화 상황에 대해 '익숙해질 것'이라는 성명을 발표합니다.", "현지 화폐가 {{LARGENUMBER}}:1로 화폐 개혁되면서 곳곳이 혼란에 빠졌습니다! 정부 관계자들은 시민들에게 게임을 저장하고 다시 불러오라고 권합니다. 시장은 통화 상황에 대해 '곧 익숙해질 것'이라고 밝혔습니다."}, /* 195 */
    {"<principalPortion>", "주요 부분", "원금 부분"}, /* 20 */
    {"<earlyBreakFee>", "얼리 브레이크 요금", "중도 해지 수수료"}, /* 21 */
    {"<clickToWithdrawEarly>", "조기 철수하려면 클릭", "조기 인출하려면 클릭"}, /* 26 */
    {"<taxEstimatesYear>", "{{MONTH}}년 {{YEAR}}월로 종료되는 회계연도 세금 추정액", "{{YEAR}}년 {{MONTH}}에 끝나는 회계연도의 세금 추정액"}, /* 24 */
    {"<taxStatementTitle>", "세금 계산서", "세금 명세서"}, /* 25 */
    {"<taxTimeDebtAdded>", "세금 납부 시간입니다! 3개월 후에 납부해야 할 {{MONEY}} 부채가 추가되었습니다.", "세금을 내는 달입니다! 3개월 뒤 갚아야 할 {{MONEY}} 부채가 추가되었습니다."}, /* 196 */
    {"<autoTransferConfirmed>", "자동 전송 규칙 생성", "자동 이체 규칙을 만들었습니다"}, /* 129 */
    {"<batchCostTitle>", "배치 비용", "묶음 비용"}, /* 202 */
    {"<buyUnitsTitle>", "구매 유닛", "구매 수량"}, /* 200 */
    {"<sellUnitsTitle>", "판매할 유닛", "판매 수량"}, /* 201 */
    {"<equityDistribution>", "주식 분배", "지분 분포"}, /* 27 */
    {"<stocksNoPublicShares>", "더 이상 공개 주식을 사용할 수 없습니다.", "살 수 있는 공개 주식이 더 없습니다."}, /* 28 */
    {"<stockFinancialsTitle>", "재정", "재무"}, /* 209 */
    {"<futuresExpiryMonth>", "만료 월", "만기 월"}, /* 203 */
    {"<futuresExpiryPrice>", "만료 가격", "만기 가격"}, /* 204 */
    {"<futuresMarginPrice>", "{{MARGINMONEY}} at {{COSTPERUNIT}}", "{{MARGINMONEY}} (단위당 {{COSTPERUNIT}})"}, /* 29 */
    {"<stockBoardMonthsUntilElection>", "다음 이사회 선거는 {{MONTHSYEAR}}로 진행됩니다.", "다음 이사회 선거: {{MONTHSYEAR}}"}, /* 207 */
    {"<stockBoardNominatePlayer>", "현재 선택된 캐릭터 추천", "현재 선택한 캐릭터 지명"}, /* 206 */
    {"<stockBoardClearPlayer>", "선수 후보 지우기", "지명한 후보 지우기"}, /* 205 */
    {"<stockBoardDeparted>", "{{PERSONNAME}}이 {{COMPANYNAME}}의 이사회에서 탈퇴했습니다.", "{{COMPANYNAME}} 이사회에서 {{PERSONNAME}} 퇴임"}, /* 208 */
    {"<stockBoardServing>", "이 캐릭터는 이사회에서 봉사하고 있습니다.", "이 캐릭터는 이사로 활동 중입니다."}, /* 30 */
    {"<personQualifiedLevel>", "{{PERSONNAME}}은(는) {{PERCENT}}% 자격 충족", "{{PERSONNAME}}: 자격 {{PERCENT}}% 충족"}, /* 131 */
    {"<qualifiedPercent>", "귀하는 {{PERCENT}}% 자격 충족", "자격 충족률 {{PERCENT}}%"}, /* 130 */
    {"<gainedStatus>", "{{PERSON}}이(가) {{STATUS}} 획득", "{{PERSON}}: {{STATUS}} 획득"}, /* 210 */
    {"<reducedStatus>", "{{PERSON}}이(가) {{STATUS}} 시간 감소", "{{PERSON}}: {{STATUS}} 시간 감소"}, /* 211 */
    {"<deathCause>", "{{DEADPERSON}}이(가) {{STATUSEFFECT}}로 사망했습니다!", "{{DEADPERSON}} 사망, 원인: {{STATUSEFFECT}}!"}, /* 212 */
    {"<alreadyQueuedPerson>", "이미 대기열에 있습니다 {{PERSONNAME}}.", "{{PERSONNAME}}의 대기열에 이미 있음"}, /* 31 */
    {"<personalityPreferences>", "{{PERSONNAME}}의 선호도가", "{{PERSONNAME}}의 선호도"}, /* 32 */
    {"<personPregnant>", "{{PERSONNAME}}이/가 임신했습니다!", "{{PERSONNAME}}: 임신했습니다!"}, /* 213 */
    {"<transportModeSelect>", "운송 모드 선택", "이동 수단 선택"}, /* 139 */
    {"<weightingActionNeedsTip>", "행동이 필요를 얼마나 회복하는지에 따라 욕망에 영향을 줍니다.", "활동이 욕구를 얼마나 채워 주는지에 따라 하고 싶은 정도에 영향을 줍니다."}, /* 181 */
    {"<weightingPotentialStatusTip>", "나쁜 잠재 상태 확률이 높을 때 욕망에 영향을 미치며, 행동으로 욕망이 더 증가합니다.", "나쁜 잠재 상태의 확률이 높은데 이 활동이 그 확률을 더 높일 때, 하고 싶은 정도에 영향을 줍니다."}, /* 182 */
    {"<settingsCopied>", "설정 복사", "설정을 복사했습니다"}, /* 215 */
    {"<pasteSettings>", "붙여넣기 설정", "설정 붙여넣기"}, /* 214 */
    {"<stopNeedsDecay>", "이 행동 정지는 부패가 필요합니다", "이 활동을 하는 동안 욕구가 줄지 않습니다"}, /* 13 */
    {"<goalOutcomeBigSuccess>", "그랜드 성공", "대성공"}, /* 216 */
    {"<goalOutcomeSmallSuccess>", "사소한 성공", "작은 성공"}, /* 217 */
    {"<goalOutcomeSmallFail>", "사소한 좌절", "작은 차질"}, /* 218 */
    {"<goalOutcomeMinimumTip>", "이 작업을 완료할 때까지 결과를 확인할 수 없습니다.", "이 활동을 마치기 전에는 이 결과가 나오지 않습니다."}, /* 219 */
    {"<meetingPeopleEnabledTip>", "캐릭터가 여기에 나타납니다. 소셜 환경 설정으로 활동을 수행하여 사람들을 만날 기회를 얻으세요.", "캐릭터가 여기에 나타납니다. 소셜 선호가 붙은 활동을 하면 사람을 만날 기회가 생깁니다."}, /* 19 */
    {"<surrenderChildAdoption>", "{{PERSONNAME}}을 입양을 위해 정말 포기하시겠습니까? 모든 법적 권리를 포기하고 다시는 만나지 못하게 됩니다.", "{{PERSONNAME}}을(를) 정말 입양 보내시겠습니까? 모든 법적 권리를 잃고 다시는 만나지 못하게 됩니다."}, /* 141 */
    {"<minimumCompatibility>", "최소 호환성", "최소 궁합"}, /* 222 */
    {"<compatibleYearsMinimum>", "젊은 (년)", "연하 (년)"}, /* 220 */
    {"<compatibleYearsMaximum>", "이전 (년)", "연상 (년)"}, /* 221 */
    {"<relationClearAll>", "관계 정리", "관계 모두 지우기"}, /* 226 */
    {"<procreationTip>", "자식 형성\r\n\r\n활성화된 경우:\r\n* 매월 말에 자식 형성을 시도합니다.", "아이 갖기\r\n\r\n활성화된 경우:\r\n* 매월 말에 아이 갖기를 시도합니다."}, /* 140 */
    {"<opinionTitle>", "의견", "호감도"}, /* 223 */
    {"<kinType3>", "로맨틱한 관계", "연애 관계"}, /* 224 */
    {"<disableBioRelated>", "생물학적 관련", "혈연 관계"}, /* 225 */
    {"<jobsTitle>", "채용", "직업"}, /* 233 */
    {"<jobPromotionTitle>", "프로모션", "승진"}, /* 33 */
    {"<casualTitle>", "캐주얼", "임시직"}, /* 227 */
    {"<casualTip>", "캐주얼. 운영 시간은 매월 달라질 수 있습니다.", "임시직. 근무 시간은 매월 달라질 수 있습니다."}, /* 228 */
    {"<nextPerformanceReview>", "다음 리뷰", "다음 평가"}, /* 232 */
    {"<minimumPerformance>", "최소 성능", "최소 실적"}, /* 229 */
    {"<afterChangePerformance>", "새로운 성능", "변경 후 실적"}, /* 230 */
    {"<jobSearchFindWorkTip>", "새 작품 검색", "새 일자리 찾기"}, /* 34 */
    {"<workFiredFromJob>", "{{PERSONNAME}}은/는 {{JOBNAME}} 직위에서 해고되었습니다.", "{{PERSONNAME}}: {{JOBNAME}}에서 해고됨"}, /* 234 */
    {"<jobNoNormalJobs>", "{{PERSONNAME}}은/는 현재 소속된 직장이 없습니다.", "{{PERSONNAME}}: 소속된 직장 없음"}, /* 235 */
    {"<personGotJob>", "{{PERSONNAME}}이/가 일자리를 얻었습니다!", "{{PERSONNAME}}: 일자리를 얻었습니다!"}, /* 236 */
    {"<personNoGotJob>", "{{PERSONNAME}}이/가 일자리를 얻지 못했습니다.", "{{PERSONNAME}}: 일자리를 얻지 못했습니다."}, /* 237 */
    {"<personQuitJob>", "{{PERSONNAME}}이/가 직장을 그만뒀습니다.", "{{PERSONNAME}}: 직장을 그만뒀습니다."}, /* 238 */
    {"<performanceReviewTooLow>", "성능이 너무 낮음", "실적이 너무 낮음"}, /* 231 */
    {"<firmPossibleAssets>", "가능한 에셋 태그", "가능한 자산 태그"}, /* 39 */
    {"<firmDesiredAssets>", "욕망 자산 태그", "필요한 자산 태그"}, /* 40 */
    {"<firmJobHoursTitle>", "도보 이동 및 계약직으로 일할 수 있는 시간.\r\n\r\n월말에 근무 시간이 불완전하면 평판이 떨어집니다.", "유동인구와 계약에서 생기는 작업 시간.\r\n\r\n월말까지 작업 시간을 다 채우지 못하면 평판이 떨어집니다."}, /* 38 */
    {"<firmFootTrafficTitle>", "도보 교통", "유동인구"}, /* 37 */
    {"<contractPayouts>", "계약금 지급", "계약 대금 지급"}, /* 242 */
    {"<signedOn>", "로그인", "체결일"}, /* 35 */
    {"<completeOn>", "완료 켜기", "완료일"}, /* 36 */
    {"<employeeQuitWage>", "({{FIRMNAME}}, {{JOBTITLE}}, {{MONTHWAGE}}, {{MONTHHOURS}}) {{STAFFNAME}}가 임금 인상 요청을 무시당해 퇴사했습니다!", "({{FIRMNAME}}, {{JOBTITLE}}, {{MONTHWAGE}}, {{MONTHHOURS}}) {{STAFFNAME}}: 임금 인상 요청이 무시되어 퇴사했습니다!"}, /* 243 */
    {"<awarenessTitle>", "인식", "인지도"}, /* 143 */
    {"<spawnWeightings>", "스폰 가중치", "등장 가중치"}, /* 241 */
    {"<businessClosed>", "닫힘", "영업 종료"}, /* 240 */
    {"<businessOpen>", "열기", "영업 중"}, /* 239 */
    {"<sortFloorspace>", "평수별 정렬", "바닥 면적별 정렬"}, /* 247 */
    {"<sortValueFloorspace>", "평수별 값으로 정렬", "바닥 면적당 가치별 정렬"}, /* 248 */
    {"<tenantAcceptPrice>", "테넌트가 갱신 제안을 수락합니다.", "임차인이 갱신 제안을 수락했습니다."}, /* 44 */
    {"<onMarket>", "마켓", "매물로 나옴"}, /* 244 */
    {"<forRent>", "임대용", "임대 매물"}, /* 245 */
    {"<rentalLandlordTaken>", "집주인이 {{HOUSEADDRESS}}를 점유했습니다!", "집주인이 {{HOUSEADDRESS}} 건물을 되찾았습니다!"}, /* 246 */
    {"<propertyForRentOfferTip>", "이 숙소를 임대할 수 있도록 설정했습니다. 이 가격과 임대 기간에 동의하거나 무시하고 다음 달에 다른 오퍼를 기다릴 수 있습니다.", "이 부동산을 임대할 수 있도록 설정했습니다. 이 가격과 임대 기간에 동의하거나 무시하고 다음 달에 다른 제안을 기다릴 수 있습니다."}, /* 144 */
    {"<currentHomeTip>", "이곳은 현재 귀하의 집입니다.", "지금 살고 있는 집입니다."}, /* 132 */
    {"<businessOccupiedTip>", "이곳은 귀하의 사업체 중 하나가 점유하고 있습니다.", "내 사업체가 쓰고 있는 곳입니다."}, /* 133 */
    {"<playerOwnProperty>", "이 부동산은 귀하가 소유하고 있습니다.", "내가 소유한 부동산입니다."}, /* 134 */
    {"<playerRentingProperty>", "이 부동산은 귀하가 임대하고 있습니다.", "내가 임차해 쓰는 부동산입니다."}, /* 135 */
    {"<itemTagsTitle>", "에셋 태그", "자산 태그"}, /* 41 */
    {"<needItemsTagTitle>", "태그가 있는 에셋 사용", "태그가 있는 자산 사용"}, /* 43 */
    {"<spawnItemsTitle>", "에셋 스폰", "자산 생성"}, /* 42 */
    {"<expectedLifeTimeTitle>", "라이프타임", "수명"}, /* 249 */
    {"<expectedUsageTimeTitle>", "사용법", "사용량"}, /* 250 */
    {"<decayOutput>", "붕괴 출력", "소멸 시 산출물"}, /* 252 */
    {"<stolenItemsChance>", "훔친 자산 확률", "자산 도난 확률"}, /* 45 */
    {"<vehicleSpeedModifier>", "차량 속도 조절기", "차량 속도 보정"}, /* 253 */
    {"<purchaseBusiness>", "이 상품은 귀하의 비즈니스를 위해 구매됩니다.", "이 상품은 내 사업체용으로 구매됩니다."}, /* 136 */
    {"<minimumEduPerformance>", "성적이 25 이하로 떨어지면 퇴장당할 위험이 있습니다.", "성적이 25 아래로 떨어지면 퇴학당할 위험이 있습니다."}, /* 46 */
    {"<householdItemsTitle>", "가정용품", "가계 자산"}, /* 254 */
    {"<personalItemsTitle>", "개인 아이템", "개인 자산"}, /* 255 */
    {"<influenceImpact>", "영향력 영향력", "영향력 효과"}, /* 145 */
    {"<policyCurrentSupportTip>", "제안서 통과 확률", "제안이 통과될 확률"}, /* 257 */
    {"<rivalMembersTitle>", "회원", "구성원"}, /* 256 */
    {"<householdMemberDesc>", "이 캐릭터는 귀하의 가구에 속합니다.", "이 캐릭터는 내 가구에 속합니다."}, /* 137 */
    {"<chartExperienceTitle>", "{{PERSONNAME}}의 경력 명세서", "{{PERSONNAME}}의 경험 명세서"}, /* 258 */
    {"<chartLifetime>", "라이프타임", "평생"}, /* 251 */
    {"<modTypeHousehold>", "가정용 모드", "가족 모드"}, /* 263 */
    {"<saveNameEntry>", "여기에 이름 저장", "저장할 이름 입력"}, /* 260 */
    {"<minorPrefTip>", "이것은 사소한 기본 설정입니다.", "작은 선호 항목입니다."}, /* 16 */
    {"<majorPrefTip>", "주요 환경 설정", "큰 선호 항목입니다."}, /* 17 */
    {"<clickToRandomisePrefs>", "클릭하여 환경 설정 무작위 지정", "클릭하여 선호도 무작위 지정"}, /* 18 */
    {"<setNewValue>", "세트", "설정"}, /* 259 */
    {"<usdOnly>", "미국 전용", "미국 달러만"}, /* 261 */
    {"<setNewKeyShortcut>", "단축키를 누르면 바로 가기", "단축키로 쓸 새 키를 누르세요"}, /* 262 */
    {"<keyZoomIn>", "줌인", "확대"}, /* 264 */
    {"<keyOpenSheetStatus>", "오픈 상태", "상태 시트 열기"}, /* 265 */
    {"<keyOpenSheetXP>", "XP 시트 열기", "경험 시트 열기"}, /* 266 */
    {"<keySpeedUp>", "속도 향상", "속도 올리기"}, /* 267 */
    {"<keySpeedDown>", "속도 감소", "속도 내리기"}, /* 268 */
    {"<steamWorkshopIDTitle>", "작업장 ID", "창작마당 ID"}, /* 47 */
    {"<exportedMoney>", "수출한 돈", "내보낸 돈"}, /* 48 */
    {"<householdExportComplete>", "가구 수출", "가구를 내보냈습니다"}, /* 49 */
    {"<siteClosedDown>", "{{SITENAME}}이(가) {{LOCATIONNAME}}에서 문을 닫았습니다", "{{LOCATIONNAME}}의 {{SITENAME}} 폐업"}, /* 269 */
    {"<foundSites>", "사이트 찾기", "찾은 사이트"}, /* 175 */
    {"<potentialStatusTitle>", "잠재력 상태", "잠재적 상태"}, /* 146 */
    {"<oldPotentialStatus>", "이전 잠재력 상태", "이전 잠재적 상태"}, /* 147 */
    {"<healthPointsTip>", "생일에는 체력 포인트가 소모됩니다.\r\n\r\n식량이나 에너지가 0에 도달하면 포인트를 잃게 됩니다.\r\n\r\n건강한 활동을 하면 포인트를 획득할 수 있습니다.", "생일에는 건강 포인트가 쓰입니다.\r\n\r\n식량이나 에너지가 0이 되면 포인트를 잃습니다.\r\n\r\n건강한 활동을 하면 포인트를 얻습니다."}, /* 271 */
    {"<happinessPointsTip>", "행복 포인트는 생일에 사용됩니다.\r\n\r\n필요량이 0에 도달하면 포인트를 잃게 됩니다.\r\n\r\n일부 욕구 막대(예: 재미)를 넘기면 포인트를 얻습니다.", "행복 포인트는 생일에 쓰입니다.\r\n\r\n욕구가 0이 되면 포인트를 잃습니다.\r\n\r\n일부 욕구 막대(예: 재미)를 넘치게 채우면 포인트를 얻습니다."}, /* 270 */
};
static const piece KO_DATA_ACTIVITIES_TRANSLATED[] = {
    {"<id>70006</id>", "name=\"아이들링\"", "name=\"멍하니 있기\""}, /* 290 */
    {"<id>70014</id>", "name=\"대중 교통\"", "name=\"대중교통\""}, /* 288 */
    {"<id>70021</id>", "name=\"사이클링\"", "name=\"자전거 타기\""}, /* 289 */
    {"<id>70321</id>", "name=\"종료\"", "name=\"그만두기\""}, /* 51 */
    {"<id>71006</id>", "name=\"서명 양식\"", "name=\"서류에 서명\""}, /* 52 */
    {"<id>71121</id>", "name=\"예금 증명서\"", "name=\"정기 예금\""}, /* 277 */
    {"<id>71136</id>", "name=\"자동 전송 관리\"", "name=\"자동 이체 관리\""}, /* 148 */
    {"<id>71147</id>", "name=\"상인의 대출\"", "name=\"고리대금\""}, /* 54 */
    {"<id>71251</id>", "name=\"이동\"", "name=\"이사 나가기\""}, /* 53 */
    {"<id>71281</id>", "name=\"영구적으로 닫기\"", "name=\"폐업하기\""}, /* 276 */
    {"<id>71516</id>", "name=\"철수\"", "name=\"인출\""}, /* 50 */
    {"<id>71731</id>", "name=\"임대 중단\"", "name=\"임대 계약 해지\""}, /* 275 */
    {"<id>71791</id>", "name=\"판매용 마크\"", "name=\"매물로 내놓기\""}, /* 273 */
    {"<id>71801</id>", "name=\"임대용 마크\"", "name=\"임대로 내놓기\""}, /* 274 */
    {"<id>71811</id>", "name=\"대여 제안에 동의\"", "name=\"임대 제안 수락\""}, /* 293 */
    {"<id>72206</id>", "name=\"쓰레기통 청소\"", "name=\"쓰레기통 뒤지기\""}, /* 55 */
    {"<id>74541</id>", "name=\"낯선 사람의 애완견\"", "name=\"낯선 사람의 개 쓰다듬기\""}, /* 284 */
    {"<id>74546</id>", "name=\"길고양이 애완동물\"", "name=\"길고양이 쓰다듬기\""}, /* 285 */
    {"<id>74571</id>", "name=\"플레이 풀\"", "name=\"당구 치기\""}, /* 286 */
    {"<id>74631</id>", "name=\"자원 봉사 정원 가꾸기\"", "name=\"정원 가꾸기 봉사\""}, /* 291 */
    {"<id>75021</id>", "name=\"다른 사람과 함께 플레이\"", "name=\"다른 사람과 놀기\""}, /* 287 */
    {"<id>75661</id>", "name=\"종교 텍스트 공부\"", "name=\"경전 공부하기\""}, /* 292 */
    {"<id>76011</id>", "name=\"콜드 콜 낯선 사람\"", "name=\"낯선 사람에게 영업 전화\""}, /* 149 */
    {"<id>76051</id>", "name=\"터무니없는 클릭 미끼 게시\"", "name=\"터무니없는 낚시성 글 올리기\""}, /* 150 */
    {"<id>76121</id>", "name=\"리프트 덤벨\"", "name=\"덤벨 들기\""}, /* 62 */
    {"<id>76231</id>", "name=\"체험판 신청하기\"", "name=\"시승 신청하기\""}, /* 63 */
    {"<id>76281</id>", "name=\"클로 머신 플레이\"", "name=\"인형 뽑기\""}, /* 64 */
    {"<id>77611</id>", ">정신적 붕괴를 얻을 확률이 감소하고 현재 정신적 붕괴를 제거할 확률이 감소합니다.</tip>", ">정신적 붕괴에 걸릴 확률이 줄고, 현재 걸린 정신적 붕괴가 사라질 확률이 조금 있습니다.</tip>"}, /* 127 */
    {"<id>78011</id>", "name=\"연금 모으기\"", "name=\"연금 받기\""}, /* 68 */
    {"<id>78031</id>", "name=\"장학금 모으기\"", "name=\"장학금 받기\""}, /* 69 */
    {"<id>80156</id>", "name=\"아트 작품 표시\"", "name=\"미술 작품 전시\""}, /* 283 */
    {"<id>80166</id>", "name=\"스케치 개요\"", "name=\"밑그림 그리기\""}, /* 281 */
    {"<id>80176</id>", "name=\"캔버스에 페인트\"", "name=\"캔버스에 그림 그리기\""}, /* 282 */
    {"<id>80226</id>", "name=\"울타리 도난품\"", "name=\"장물 처분\""}, /* 65 */
    {"<id>80246</id>", "name=\"계획 강도\"", "name=\"강도 계획 세우기\""}, /* 66 */
    {"<id>80251</id>", "name=\"스카우트 탈출 경로\"", "name=\"탈출 경로 정찰\""}, /* 67 */
    {"<id>80346</id>", "name=\"실 기업 제품\"", "name=\"기업 제품 바람잡이\""}, /* 57 */
    {"<id>80531</id>", "name=\"잼 온 기타\"", "name=\"기타로 즉흥 연주\""}, /* 278 */
    {"<id>80571</id>", "name=\"데뷔 송 온라인\"", "name=\"온라인에 데뷔곡 공개\""}, /* 279 */
    {"<id>80581</id>", "name=\"노래 실력 향상\"", "name=\"노래 실력 홍보\""}, /* 280 */
    {"<id>80751</id>", "name=\"고용 빌더\"", "name=\"건축업자 고용\""}, /* 152 */
    {"<id>80758</id>", "name=\"바로 가기\"", "name=\"지름길 택하기\""}, /* 56 */
    {"<id>80931</id>", "name=\"디페이스 공공 예술\"", "name=\"공공 미술품 훼손\""}, /* 155 */
    {"<id>80951</id>", "name=\"랠리 서포터\"", "name=\"지지자 결집\""}, /* 156 */
    {"<id>80971</id>", "name=\"프로모션 이벤트\"", "name=\"행사 홍보\""}, /* 157 */
    {"<id>81241</id>", "name=\"연구 호환성\"", "name=\"호환성 조사\""}, /* 151 */
    {"<id>81321</id>", "name=\"플랜트 시작\"", "name=\"식물 심기\""}, /* 58 */
    {"<id>81331</id>", "name=\"플랜트 모니터링\"", "name=\"식물 살피기\""}, /* 59 */
    {"<id>81396</id>", "name=\"조사 쓰기\"", "name=\"탐사 기사 쓰기\""}, /* 60 */
    {"<id>81421</id>", "name=\"법정에서 법에 도전하세요\"", "name=\"법정에서 법에 맞서기\""}, /* 61 */
};
static const piece KO_DATA_BUSINESS_TRANSLATED[] = {
    {"<id>40121</id>", "name=\"은퇴자 홈\"", "name=\"양로원\""}, /* 294 */
    {"<id>40221</id>", "name=\"보안 계약자\"", "name=\"경비 용역업체\""}, /* 295 */
    {"<id>40311</id>", "name=\"정원 보육원\"", "name=\"묘목원\""}, /* 70 */
    {"<id>40361</id>", "name=\"가공 상점\"", "name=\"기계 가공 공장\""}, /* 296 */
    {"<id>40411</id>", "name=\"음악 학교\"", "name=\"음악 학원\""}, /* 297 */
};
static const piece KO_DATA_CITIES_TRANSLATED[] = {
    {"<id>55991</id>", "인생의 많은 웅장한 경험은 제공되지 않습니다.", "인생의 더 큰 경험은 대부분 빠져 있습니다."}, /* 337 */
};
static const piece KO_DATA_CONTRACTS_TRANSLATED[] = {
    {"<id>35051</id>", "name=\"공급 회사\"", "name=\"기업에 납품\""}, /* 158 */
    {"<id>35056</id>", "name=\"공급망 상점\"", "name=\"체인점에 납품\""}, /* 162 */
    {"<id>35066</id>", "name=\"보급품 레스토랑\"", "name=\"레스토랑에 납품\""}, /* 159 */
    {"<id>35071</id>", "name=\"정부 공급\"", "name=\"정부에 납품\""}, /* 163 */
    {"<id>35086</id>", "name=\"공급 대학\"", "name=\"대학에 납품\""}, /* 160 */
    {"<id>35091</id>", "name=\"보급 병원\"", "name=\"병원에 납품\""}, /* 161 */
    {"<id>35411</id>", "name=\"기업 함대 유지\"", "name=\"기업 차량 정비\""}, /* 71 */
};
static const piece KO_DATA_EDUCATIONS_TRANSLATED[] = {
    {"<id>45026</id>", ">초등학교 7학년 및 8학년. 11~13세 어린이에게 적합합니다.</tip>", ">학교 7, 8학년. 11~13세 어린이에게 적합합니다.</tip>"}, /* 72 */
    {"<id>45031</id>", ">초등학교 9학년 및 10학년. 13~15세 어린이에게 적합합니다.</tip>", ">학교 9, 10학년. 13~15세 어린이에게 적합합니다.</tip>"}, /* 73 */
    {"<id>45036</id>", ">초등학교 11학년 및 12학년. 15세 이상에게 적합합니다.</tip>", ">학교 11, 12학년. 15세 이상에게 적합합니다.</tip>"}, /* 74 */
    {"<id>45121</id>", "name=\"마스터: 교육\"", "name=\"석사: 교육학\""}, /* 75 */
    {"<id>45511</id>", "name=\"인증서: 요리\"", "name=\"자격증: 요리\""}, /* 298 */
    {"<id>46011</id>", "name=\"마스터: 비즈니스 관리자.\"", "name=\"석사: 경영학\""}, /* 76 */
    {"<id>47021</id>", "name=\"영향력 있는 사람 101\"", "name=\"사람을 움직이는 법 101\""}, /* 77 */
};
static const piece KO_DATA_JOBS_TRANSLATED[] = {
    {"<id>30101</id>", "name=\"캐셔\"", "name=\"계산원\""}, /* 301 */
    {"<id>30121</id>", "name=\"접수\"", "name=\"접수 담당\""}, /* 81 */
    {"<id>30301</id>", "name=\"베이커\"", "name=\"제빵사\""}, /* 302 */
    {"<id>30351</id>", "name=\"요리\"", "name=\"요리사\""}, /* 80 */
    {"<id>30401</id>", "name=\"부기\"", "name=\"경리\""}, /* 78 */
    {"<id>30511</id>", "name=\"식기 세척기\"", "name=\"설거지 담당\""}, /* 79 */
    {"<id>30601</id>", "name=\"배달 드라이버\"", "name=\"배달 기사\""}, /* 303 */
    {"<id>30811</id>", "name=\"상주 의사\"", "name=\"레지던트\""}, /* 83 */
    {"<id>31341</id>", "name=\"지구 판사\"", "name=\"지방법원 판사\""}, /* 84 */
    {"<id>31631</id>", "name=\"재정 고문\"", "name=\"재무 상담사\""}, /* 300 */
    {"<id>31661</id>", ">매우 긴 시간.</tip>", ">근무 시간이 매우 깁니다.</tip>"}, /* 336 */
    {"<id>32211</id>", "name=\"음악 튜터\"", "name=\"음악 강사\""}, /* 304 */
    {"<id>32371</id>", "name=\"빌더 견습생\"", "name=\"건축 견습생\""}, /* 153 */
    {"<id>32381</id>", "name=\"빌더\"", "name=\"건축업자\""}, /* 154 */
    {"<id>32431</id>", "name=\"계정 관리자\"", "name=\"고객 담당 매니저\""}, /* 299 */
    {"<id>32521</id>", "name=\"감사\"", "name=\"회계 감사원\""}, /* 82 */
};
static const piece KO_DATA_MISC_TAGS2[] = {
    {"<id>1281</id>", "name=\"기타 디렉터십\"", "name=\"다른 이사직\""}, /* 318 */
};
static const piece KO_DATA_MISC_TRANSLATED[] = {
    {"<id>1818</id>", "name=\"공개 지원\"", "name=\"대중의 지지\""}, /* 319 */
    {"<id>1831</id>", "name=\"인생은 웅장하다!\"", "name=\"멋진 인생!\""}, /* 320 */
    {"<id>2065</id>", "name=\"재고 주문\"", "name=\"주식 주문\""}, /* 108 */
    {"<id>4006</id>", "name=\"충분히 소유했나요?\"", "name=\"충분히 갖고 있는가?\""}, /* 305 */
    {"<id>4011</id>", "name=\"남은 시간은 몇 시간인가요?\"", "name=\"남은 시간은?\""}, /* 306 */
    {"<id>4016</id>", "name=\"비용은 얼마인가요?\"", "name=\"비용은?\""}, /* 307 */
    {"<id>4021</id>", "name=\"얼마나 멀리 있나요?\"", "name=\"거리는?\""}, /* 308 */
    {"<id>4031</id>", "name=\"어떤 우선순위가 있나요?\"", "name=\"우선순위는?\""}, /* 112 */
    {"<id>4036</id>", "name=\"이번 달에 끝났나요?\"", "name=\"이번 달에 했는가?\""}, /* 309 */
    {"<id>4041</id>", "name=\"요구 사항은 어떻게 영향을 받나요?\"", "name=\"욕구에 어떤 영향이 있는가?\""}, /* 113 */
    {"<id>4043</id>", "name=\"잠재적 상태가 영향을 받습니까?\"", "name=\"잠재적 상태에 영향이 있는가?\""}, /* 310 */
    {"<id>4046</id>", "name=\"아기에게 도움이 필요하신가요?\"", "name=\"아기에게 도움이 필요한가?\""}, /* 311 */
    {"<id>4056</id>", "name=\"불완전한 작업\"", "name=\"미완료 작업\""}, /* 165 */
    {"<id>5026</id>", "name=\"체력 비용\"", "name=\"의료비\""}, /* 106 */
    {"<id>5046</id>", "name=\"대여 가격\"", "name=\"임대료\""}, /* 107 */
    {"<id>10055</id>", ">스릴을 추구하는 사람들과 아드레날린 중독자들은 그렇지 않습니다.</tip>", ">대부분의 사람은 편안함을 좋아하지만 스릴을 추구하는 사람과 아드레날린 중독자는 그렇지 않습니다.</tip>"}, /* 125 */
    {"<id>10060</id>", ">최신 가제트에 대한 관심과 복잡한 기계와의 상호 작용.</tip>", ">최신 기기와 복잡한 기계를 다루는 일에 대한 관심.</tip>"}, /* 171 */
    {"<id>10550</id>", "name=\"XP:닥터링\"", "name=\"XP:진료\""}, /* 321 */
    {"<id>10625</id>", "name=\"XP:역학\"", "name=\"XP:기계 정비\""}, /* 322 */
    {"<id>10630</id>", "name=\"XP:차량 역학\"", "name=\"XP:차량 정비\""}, /* 323 */
    {"<id>10640</id>", "name=\"XP:커피 양조\"", "name=\"XP:커피 추출\""}, /* 325 */
    {"<id>10700</id>", "name=\"XP:판단\"", "name=\"XP:재판\""}, /* 324 */
    {"<id>10770</id>", "name=\"XP:대학원. 감독\"", "name=\"XP:대학원생 지도\""}, /* 326 */
    {"<id>61051</id>", "name=\"콜드 콜\"", "name=\"영업 전화\""}, /* 164 */
    {"<id>66636</id>", "name=\"매들리 인 러브\"", "name=\"열렬한 사랑\""}, /* 110 */
    {"<id>66656</id>", "name=\"친구 닫기\"", "name=\"가까운 친구\""}, /* 111 */
    {"<id>68091</id>", "name=\"환승 요금\"", "name=\"대중교통 요금\""}, /* 109 */
    {"<id>86571</id>", "name=\"낮은 대중교통 보조금\"", "name=\"대중교통 보조금 인하\""}, /* 99 */
    {"<id>86591</id>", "name=\"낮은 도로 투자\"", "name=\"도로 투자 축소\""}, /* 100 */
    {"<id>86611</id>", "name=\"경찰 기금 모금\"", "name=\"경찰 예산 확대\""}, /* 102 */
    {"<id>86621</id>", "name=\"경찰 자금 지원 감소\"", "name=\"경찰 예산 축소\""}, /* 103 */
    {"<id>86741</id>", "name=\"연합 격려\"", "name=\"노동조합 장려\""}, /* 104 */
    {"<id>86751</id>", "name=\"연합 진압\"", "name=\"노동조합 탄압\""}, /* 105 */
    {"<id>86781</id>", "name=\"낮은 복지 혜택\"", "name=\"복지 혜택 축소\""}, /* 101 */
    {"<id>86806</id>", "name=\"농업 자금 지원\"", "name=\"농업 예산 삭감\""}, /* 85 */
    {"<id>86826</id>", "name=\"디펀드 에너지\"", "name=\"에너지 예산 삭감\""}, /* 86 */
    {"<id>86836</id>", "name=\"제조 자금 조달\"", "name=\"제조업 예산 삭감\""}, /* 87 */
    {"<id>86846</id>", "name=\"교통비 환불\"", "name=\"교통 예산 삭감\""}, /* 88 */
    {"<id>86856</id>", "name=\"음식 서비스 환불\"", "name=\"음식 서비스 예산 삭감\""}, /* 89 */
    {"<id>86866</id>", "name=\"소매업 환불\"", "name=\"소매업 예산 삭감\""}, /* 90 */
    {"<id>86876</id>", "name=\"건강 자금\"", "name=\"보건 예산 삭감\""}, /* 91 */
    {"<id>86886</id>", "name=\"교육 자금 지원\"", "name=\"교육 예산 삭감\""}, /* 92 */
    {"<id>86891</id>", "name=\"투자 전문 서비스\"", "name=\"전문 서비스 투자\""}, /* 98 */
    {"<id>86896</id>", "name=\"디펀드 전문 서비스\"", "name=\"전문 서비스 예산 삭감\""}, /* 93 */
    {"<id>86906</id>", "name=\"디펀드 엔터테인먼트\"", "name=\"엔터테인먼트 예산 삭감\""}, /* 94 */
    {"<id>86916</id>", "name=\"디펀드 기술\"", "name=\"기술 예산 삭감\""}, /* 95 */
    {"<id>86926</id>", "name=\"주거용 부동산 처분\"", "name=\"주거용 부동산 예산 삭감\""}, /* 96 */
    {"<id>86946</id>", "name=\"정부 자금 지원\"", "name=\"정부 예산 삭감\""}, /* 97 */
    {"<id>87031</id>", "name=\"앉은 표면\"", "name=\"앉을 수 있는 곳\""}, /* 316 */
    {"<id>87059</id>", "name=\"펀 액세스 패스\"", "name=\"놀이시설 이용권\""}, /* 314 */
    {"<id>87264</id>", "name=\"정장 착용\"", "name=\"정장\""}, /* 315 */
    {"<id>87521</id>", "name=\"EFTPOS\"", "name=\"카드 결제 단말기\""}, /* 312 */
    {"<id>87538</id>", "name=\"자료 작성\"", "name=\"필기구\""}, /* 313 */
    {"<id>87756</id>", "name=\"발굴\"", "name=\"굴착\""}, /* 317 */
};
static const piece KO_DATA_PASSIONS_TRANSLATED[] = {
    {"<id>36071</id>", "name=\"항의\"", "name=\"시위\""}, /* 114 */
    {"<id>36081</id>", ">한 번에 많은 친구들과 의견을 나누기에 좋습니다.</tip>", ">여러 친구와 한꺼번에 호감을 쌓기에 좋습니다.</tip>"}, /* 126 */
};
static const piece KO_DATA_QUESTS_TRANSLATED[] = {
    {"<id>98611</id>", "계약은 비즈니스의 제품이나 서비스를 고객에게 공급하기 위해 귀하와 고객 간에 체결하는 장기 계약입니다.", "계약은 내 사업체의 제품이나 서비스를 고객에게 공급하기로 고객과 맺는 장기 계약입니다."}, /* 138 */
};
static const piece KO_DATA_RESTATE_TRANSLATED[] = {
    {"<id>57091</id>", "name=\"호보 캠프\"", "name=\"노숙자 캠프\""}, /* 168 */
};
static const piece KO_DATA_SITES_TRANSLATED[] = {
    {"<id>50121</id>", "name=\"부유한 주거\"", "name=\"고급 주택가\""}, /* 172 */
    {"<id>50501</id>", "name=\"학년 학교\"", "name=\"초등학교\""}, /* 0 */
    {"<id>50506</id>", "name=\"무역 학교\"", "name=\"직업학교\""}, /* 1 */
    {"<id>50581</id>", "name=\"라이브러리\"", "name=\"도서관\""}, /* 2 */
    {"<id>50591</id>", "name=\"커뮤니티 홀\"", "name=\"주민 회관\""}, /* 173 */
    {"<id>51051</id>", "name=\"살롱\"", "name=\"미용실\""}, /* 174 */
};
static const piece KO_DATA_STATUS_TRANSLATED[] = {
    {"<id>12031</id>", "name=\"고갈\"", "name=\"탈진\""}, /* 116 */
    {"<id>12286</id>", "name=\"슬픔 (무모함)\"", "name=\"애도 (무모함)\""}, /* 329 */
    {"<id>12561</id>", "name=\"행오버\"", "name=\"숙취\""}, /* 115 */
    {"<id>12571</id>", "name=\"지루해\"", "name=\"지루함\""}, /* 328 */
    {"<id>12611</id>", "name=\"임신 임신기\"", "name=\"임신기\""}, /* 117 */
    {"<id>12743</id>", "name=\"당겨진 근육\"", "name=\"근육 염좌\""}, /* 167 */
    {"<id>13521</id>", "name=\"가장 가치 있는 플레이어\"", "name=\"MVP\""}, /* 124 */
    {"<id>13531</id>", "name=\"실망한 추종자\"", "name=\"실망한 팔로워\""}, /* 327 */
    {"<id>14001</id>", "name=\"속임수 죽음\"", "name=\"구사일생\""}, /* 118 */
    {"<id>14531</id>", "name=\"염증성 폐\"", "name=\"폐 염증\""}, /* 166 */
    {"<id>16021</id>", "name=\"엔돌핀 러쉬\"", "name=\"엔도르핀 폭발\""}, /* 119 */
    {"<id>16041</id>", "name=\"도파민 러쉬\"", "name=\"도파민 폭발\""}, /* 120 */
    {"<id>16071</id>", "name=\"부스트된 자아\"", "name=\"자신감 상승\""}, /* 121 */
    {"<id>16091</id>", "name=\"완료\"", "name=\"성취감\""}, /* 122 */
    {"<id>16121</id>", "name=\"만족\"", "name=\"포만감\""}, /* 123 */
};
static const piece KO_DATA_WARES_TRANSLATED[] = {
    {"<id>20361</id>", "name=\"가슴 냉동고\"", "name=\"상자형 냉동고\""}, /* 332 */
    {"<id>25141</id>", "name=\"하드웨어 소모품\"", "name=\"철물 용품\""}, /* 334 */
    {"<id>27091</id>", "name=\"프라임 무버\"", "name=\"트랙터 트럭\""}, /* 333 */
    {"<id>28181</id>", "name=\"냄비 및 용기\"", "name=\"화분과 용기\""}, /* 330 */
    {"<id>28211</id>", "name=\"원시 금속\"", "name=\"금속 원자재\""}, /* 331 */
    {"<id>29021</id>", "name=\"표\"", "name=\"테이블\""}, /* 335 */
    {"<id>29351</id>", "name=\"라이트 플레인 (전기)\"", "name=\"경비행기 (전기)\""}, /* 169 */
    {"<id>29361</id>", "name=\"라이트 플레인 (연료)\"", "name=\"경비행기 (연료)\""}, /* 170 */
};
#define RE_KO_PIECES 334
#define RE_KO_FILES \
    {RE_ADD_KOREAN, "<shortcut>바로 가기</shortcut>", KO_BASETRANSLATE, (int)(sizeof KO_BASETRANSLATE / sizeof KO_BASETRANSLATE[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"성인 대기 중\"/>", KO_DATA_ACTIVITIES_TRANSLATED, (int)(sizeof KO_DATA_ACTIVITIES_TRANSLATED / sizeof KO_DATA_ACTIVITIES_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"레모네이드 스탠드\"/>", KO_DATA_BUSINESS_TRANSLATED, (int)(sizeof KO_DATA_BUSINESS_TRANSLATED / sizeof KO_DATA_BUSINESS_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"싱가포르\"/>", KO_DATA_CITIES_TRANSLATED, (int)(sizeof KO_DATA_CITIES_TRANSLATED / sizeof KO_DATA_CITIES_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"결혼식 장소 공급\"/>", KO_DATA_CONTRACTS_TRANSLATED, (int)(sizeof KO_DATA_CONTRACTS_TRANSLATED / sizeof KO_DATA_CONTRACTS_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"학교 1,2학년\"/>", KO_DATA_EDUCATIONS_TRANSLATED, (int)(sizeof KO_DATA_EDUCATIONS_TRANSLATED / sizeof KO_DATA_EDUCATIONS_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"매장 관리자\"/>", KO_DATA_JOBS_TRANSLATED, (int)(sizeof KO_DATA_JOBS_TRANSLATED / sizeof KO_DATA_JOBS_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"현금\"/>", KO_DATA_MISC_TAGS2, (int)(sizeof KO_DATA_MISC_TAGS2 / sizeof KO_DATA_MISC_TAGS2[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"음식\"/>", KO_DATA_MISC_TRANSLATED, (int)(sizeof KO_DATA_MISC_TRANSLATED / sizeof KO_DATA_MISC_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"결혼\"/>", KO_DATA_PASSIONS_TRANSLATED, (int)(sizeof KO_DATA_PASSIONS_TRANSLATED / sizeof KO_DATA_PASSIONS_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"기본 사항\"/>", KO_DATA_QUESTS_TRANSLATED, (int)(sizeof KO_DATA_QUESTS_TRANSLATED / sizeof KO_DATA_QUESTS_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"시장 가판대\"/>", KO_DATA_RESTATE_TRANSLATED, (int)(sizeof KO_DATA_RESTATE_TRANSLATED / sizeof KO_DATA_RESTATE_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"아파트\"/>", KO_DATA_SITES_TRANSLATED, (int)(sizeof KO_DATA_SITES_TRANSLATED / sizeof KO_DATA_SITES_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"영양실조\"/>", KO_DATA_STATUS_TRANSLATED, (int)(sizeof KO_DATA_STATUS_TRANSLATED / sizeof KO_DATA_STATUS_TRANSLATED[0])}, \
    {RE_ADD_KOREAN, "<desc name=\"싱글 침대\"/>", KO_DATA_WARES_TRANSLATED, (int)(sizeof KO_DATA_WARES_TRANSLATED / sizeof KO_DATA_WARES_TRANSLATED[0])}
